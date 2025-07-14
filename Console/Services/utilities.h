//
// Created by Brett King on 17/6/2025.
//

#ifndef UTILITIES_H
#define UTILITIES_H
#include <cstdlib>
#include <filesystem>
#include <ifaddrs.h>
#include <string>
#include <net/if_dl.h>
#include <sys/socket.h>
#include <uuid/uuid.h>

#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <openssl/sha.h>

#include <future>

#include "Logger.h"

namespace comet
  {
    // Function to get the full filename and directory, creating directories if necessary

    inline std::string getFullFilenameAndDirectory(const std::string &filename)
      {
        if (filename.empty()) {
          return "";
        }

        // Check if the filename is already an absolute path
        if (filename[0] == '/' || (filename.size() > 2 && filename[1] == ':' && filename[2] == '\\')) {
          return filename; // Already an absolute path
        }

        std::string fullPath = filename;
        if (filename[0] == '~') {
          const char *home = std::getenv("HOME");
          if (home) {
            fullPath = std::string(home) + filename.substr(1);
          }
        }

        // Get the parent directory
        std::filesystem::path parentPath = std::filesystem::path(fullPath).parent_path();
        if (!std::filesystem::exists(parentPath)) {
          if (!std::filesystem::create_directories(parentPath)) {
            COMETLOG("Failed to create database directory: " + parentPath.string(), comet::LoggerLevel::CRITICAL);
            throw std::runtime_error("Unable to create database directory: " + parentPath.string());
          }
          COMETLOG("Database directory created: " + parentPath.string(), comet::LoggerLevel::INFO);
        }

        return std::filesystem::path(fullPath);
      }

    inline std::string generateTimestamp()
      {
        return std::to_string(std::time(nullptr));
      }


    inline std::string getMacAddress()
      {
#ifdef _WIN32
    IP_ADAPTER_INFO AdapterInfo[16];
    DWORD dwBufLen = sizeof(AdapterInfo);
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
    if (dwStatus != ERROR_SUCCESS) {
        return "";
    }

    PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
    std::ostringstream macStream;
    macStream << std::hex;

    while (pAdapterInfo) {
        for (UINT i = 0; i < pAdapterInfo->AddressLength; i++) {
            macStream << (i == 0 ? "" : ":") << std::setw(2) << std::setfill('0') << (int)pAdapterInfo->Address[i];
        }
        break;
    }

    return macStream.str();
#elif __APPLE__
        struct ifaddrs *ifaddr, *ifa;
        unsigned char mac[6];
        std::string macAddress = "";

        if (getifaddrs(&ifaddr) == -1) {
          perror("getifaddrs");
          return macAddress;
        }

        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
          if (ifa->ifa_addr == nullptr || ifa->ifa_addr->sa_family != AF_LINK) continue;

          struct sockaddr_dl *sdl = (struct sockaddr_dl *) ifa->ifa_addr;
#ifndef IFT_ETHER
#define IFT_ETHER 0x6
#endif
          if (sdl->sdl_type == IFT_ETHER) {
            memcpy(mac, LLADDR(sdl), 6);
            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            macAddress = std::string(macStr);
            break;
          }
        }

        freeifaddrs(ifaddr);
        return macAddress;
#elif __linux__
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return "";

    struct ifreq ifr;
    const char* iface = "eth0";
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
        unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        close(sock);
        return std::string(macStr);
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

    inline std::string generateMachineId(const std::string &mac, const std::string &uuid)
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

    inline size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
      {
        auto *str = static_cast<std::string *>(userp);
        str->append(static_cast<char *>(contents), size * nmemb);
        return size * nmemb;
      }

    inline std::string getPrivateIPAddress()
      {
        struct ifaddrs *ifaddr, *ifa;
        char host[NI_MAXHOST];
        std::string privateIP = "Not found";

        if (getifaddrs(&ifaddr) == -1) {
          perror("getifaddrs");
          return privateIP;
        }

        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
          if (ifa->ifa_addr == nullptr) continue;

          if (ifa->ifa_addr->sa_family == AF_INET) {
            // IPv4
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0,
                            NI_NUMERICHOST) == 0) {
              std::string interfaceName = ifa->ifa_name;
              if (interfaceName != "lo0") {
                // Exclude loopback interface
                privateIP = host;
                break;
              }
            }
          }
        }

        freeifaddrs(ifaddr);
        return privateIP;
      }

    inline std::string getPublicIPAddressFromWeb()
      {
        CURL *curl;
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


    inline bool check200Response(const std::string &ipaddress)
      {
        CURL *curl;
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
        } else if (res == CURLE_OPERATION_TIMEDOUT) {
          COMETLOG(std::string("curl timeout failed: ") + curl_easy_strerror(res), LoggerLevel::DEBUGGING);
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        } else {
          COMETLOG(std::string("curl failed: ") + curl_easy_strerror(res), LoggerLevel::DEBUGGING);
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return res == CURLE_OK;
      }

    inline std::vector<std::string> split(const std::string &str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter)) {
          tokens.push_back(token);
        }

        return tokens;
    }
    inline std::string lower(const std::string &str) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        return lowerStr;
    }
    inline std::string upper(const std::string &str) {
        std::string upperStr = str;
        std::transform(upperStr.begin(), upperStr.end(), upperStr.begin(), ::toupper);
        return upperStr;
    }
    
  }

#endif //UTILITIES_H
