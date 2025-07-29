//
// Created by Brett King on 17/6/2025.
//

#ifndef UTILITIES_H
#define UTILITIES_H
#include <cstdlib>
#include <filesystem>
#include <string>

#include <cstring>
#include <sys/types.h>
#include <curl/curl.h>
#include <openssl/sha.h>

#if defined(__unix__) || defined(__APPLE__)
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <sys/socket.h>
#include <uuid/uuid.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_types.h>
#include <unistd.h>
#endif

#ifdef _WIN32
  #include <winsock2.h>
  #include <iphlpapi.h>
  #pragma comment(lib, "iphlpapi.lib")
#endif

#include <future>

#include "Logger.h"

namespace comet
{
  // Function to get the full filename and directory, creating directories if necessary
  inline std::string lower(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    return lowerStr;
  }
  inline std::string upper(const std::string& str) {
    std::string upperStr = str;
    std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
    return upperStr;
  }

  inline std::string getFullFilenameAndDirectory(const std::string& filename)
  {
    if (filename.empty()) {
      return "";
    }

    std::string fullPath = filename;

    // Expand ~ on Unix/Mac or fallback on Windows
    if (filename[0] == '~') {
#if defined(_WIN32)
      const char* home = std::getenv("USERPROFILE");
#else
      const char* home = std::getenv("HOME");
#endif
      if (home) {
        fullPath = std::string(home) + filename.substr(1);
      }
    }

    std::filesystem::path path(fullPath);

    // If it's not already absolute, make it relative to current path
    if (!path.is_absolute()) {
      path = std::filesystem::absolute(path);
    }

    std::filesystem::path parentPath = path.parent_path();
    if (!std::filesystem::exists(parentPath)) {
      if (!std::filesystem::create_directories(parentPath)) {
        COMETLOG("Failed to create directory: " + parentPath.string(), comet::LoggerLevel::CRITICAL);
        throw std::runtime_error("Unable to create directory: " + parentPath.string());
      }
      COMETLOG("Directory created: " + parentPath.string(), comet::LoggerLevel::INFO);
    }

    return path.string();
  }

  inline std::string generateTimestamp()
  {
    return std::to_string(std::time(nullptr));
  }

  inline std::string getMacAddress()
  {
#ifdef _WIN32
    IP_ADAPTER_INFO adapterInfo[16];  // enough for most machines
    DWORD bufLen = sizeof(adapterInfo);

    DWORD status = GetAdaptersInfo(adapterInfo, &bufLen);
    if (status != ERROR_SUCCESS) {
      return "";
    }

    PIP_ADAPTER_INFO pAdapterInfo = adapterInfo;
    std::ostringstream macStream;
    macStream << std::hex;

    // Get the MAC of the first adapter
    for (UINT i = 0; i < pAdapterInfo->AddressLength; ++i) {
      if (i > 0)
        macStream << ":";
      macStream << std::setw(2) << std::setfill('0') << static_cast<int>(pAdapterInfo->Address[i]);
    }

    return macStream.str();

#elif defined(__APPLE__)
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <cstring>
    struct ifaddrs* ifaddr, * ifa;
    unsigned char mac[6];
    std::string macAddress;

    if (getifaddrs(&ifaddr) == -1) {
      return "";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_LINK)
        continue;

      struct sockaddr_dl* sdl = reinterpret_cast<struct sockaddr_dl*>(ifa->ifa_addr);
      if (sdl->sdl_type == IFT_ETHER) {
        std::memcpy(mac, LLADDR(sdl), 6);
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        macAddress = macStr;
        break;
      }
    }

    freeifaddrs(ifaddr);
    return macAddress;

#elif defined(__linux__)
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <cstring>
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return "";

    struct ifreq ifr;
    const char* iface = "eth0";
    std::strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
      unsigned char* mac = reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);
      char macStr[18];
      snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      close(sock);
      return macStr;
    }

    close(sock);
    return "";

#else
    return "Unsupported platform";
#endif
  }

  inline std::string getUuid()
  {
#ifdef _WIN32
    UUID uuid;
    UuidCreate(&uuid);
    RPC_CSTR uuidStr;
    UuidToStringA(&uuid, &uuidStr);
    std::string result((char*)uuidStr);
    RpcStringFreeA(&uuidStr);
    return result;
#elif __APPLE__ || __linux__
    uuid_t uuid;
    uuid_generate(uuid);
    char uuidStr[37];
    uuid_unparse_lower(uuid, uuidStr);
    return std::string(uuidStr);
#else
    return "Unsupported platform";
#endif
  }

  inline std::string generateMachineId(const std::string& mac, const std::string& uuid)
  {
    std::string input = mac + uuid;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input.c_str(), input.length());
    SHA256_Final(hash, &sha256);
    std::string output;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
      char buf[3];
      snprintf(buf, sizeof(buf), "%02x", hash[i]);
      output += buf;
    }
    return output;
  }

  inline size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
  {
    auto* str = static_cast<std::string*>(userp);
    str->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
  }

  inline std::string getMachineName() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
      throw std::runtime_error("Failed to retrieve machine name");
    }
    return lower(std::string(hostname));
  }

  inline std::string getPrivateIPAddress()
  {
#ifdef _WIN32
    char ac[NI_MAXHOST] = { 0 };

    // Initialize Winsock (required before using getaddrinfo etc.)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
      return "WSAStartup failed";

    char hostname[NI_MAXHOST];
    if (gethostname(hostname, NI_MAXHOST) != 0) {
      WSACleanup();
      return "Failed to get hostname";
    }

    addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    addrinfo* info = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &info) != 0) {
      WSACleanup();
      return "Failed to resolve host IP";
    }

    for (addrinfo* p = info; p != nullptr; p = p->ai_next) {
      void* addr = &((sockaddr_in*)p->ai_addr)->sin_addr;
      inet_ntop(p->ai_family, addr, ac, sizeof(ac));

      if (std::string(ac) != "127.0.0.1") {
        std::string result = ac;
        freeaddrinfo(info);
        WSACleanup();
        return result;
      }
    }

    freeaddrinfo(info);
    WSACleanup();
    return "Not found";

#else
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;
    char host[NI_MAXHOST] = { 0 };
    std::string privateIP = "Not found";

    if (getifaddrs(&ifaddr) == -1) {
      return "getifaddrs failed";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
      if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
        continue;

      if (getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) != 0)
        continue;

      std::string ip = host;
      std::string name = ifa->ifa_name;

      if (ip != "127.0.0.1" && name != "lo" && name != "lo0") {
        privateIP = ip;
        break;
      }
    }

    freeifaddrs(ifaddr);
    return privateIP;
#endif
  }

  inline std::string getPublicIPAddressFromWeb()
  {
    CURL* curl;
    CURLcode res;
    std::string publicIP = "";

    curl_global_init(CURL_GLOBAL_DEFAULT); // Global initialization

    curl = curl_easy_init();
    if (!curl) {
      COMETLOG("Failed to initialize CURL", LoggerLevel::CRITICAL);
      curl_global_cleanup();
      return publicIP;
    }

    curl_easy_setopt(curl, CURLOPT_URL, "http://api.ipify.org");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &publicIP);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      COMETLOG(std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res), LoggerLevel::CRITICAL);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup(); // Global cleanup

    return publicIP;
  }

  inline std::string getMachineId()
  {
    std::string mac = getMacAddress();
    if (mac.empty()) {
      COMETLOG("Failed to retrieve MAC address", LoggerLevel::CRITICAL);

      return "";
    }

    std::string uuid = getUuid();
    if (uuid.empty()) {
      COMETLOG("Failed to generate UUID", LoggerLevel::CRITICAL);
      return "";
    }
    return mac;
  }


  inline bool check200Response(const std::string& ipaddress)
  {
    CURL* curl;
    CURLcode res;
    long httpCode = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
      COMETLOG("Failed to initialize CURL", LoggerLevel::CRITICAL);
      curl_global_cleanup();
      return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, ipaddress.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 2L);

    res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
      COMETLOG(std::string("Found IP Address: " + ipaddress), LoggerLevel::INFO);
    }
    else if (res == CURLE_OPERATION_TIMEDOUT) {
      COMETLOG(std::string("curl timeout failed: ") + curl_easy_strerror(res), LoggerLevel::DEBUGGING);
      curl_easy_cleanup(curl);
      curl_global_cleanup();
      return false;
    }
    else {
      COMETLOG(std::string("curl failed: ") + curl_easy_strerror(res), LoggerLevel::DEBUGGING);
      curl_easy_cleanup(curl);
      curl_global_cleanup();
      return false;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return res == CURLE_OK;
  }

  inline std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
      tokens.push_back(token);
    }

    return tokens;
  }

  inline std::string jsEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
      if (c == '\\') out += "\\\\";
      else if (c == '\'') out += "\\'";
      else out += c;
    }
    return out;
  }

  inline bool isWindows() {
#ifdef _WIN32
    return true;
#else
    return false;
#endif
  }

  inline std::string convertToOSFormat(std::string mixedPath)
  {
    std::string convertedString = mixedPath;
    if (isWindows()) {
      // On Windows, convert all '/' to '\'
      std::replace(convertedString.begin(), convertedString.end(), '/', '\\');
    }
    else
    {
      // On Unix/Mac, convert all '\' to '/'
      std::replace(convertedString.begin(), convertedString.end(), '\\', '/');

    }
    return convertedString;
  }
}

#endif //UTILITIES_H
