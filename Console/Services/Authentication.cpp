//
// Created by Brett King on 17/6/2025.
//
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <nlohmann/json.hpp>
#include "Curl.hpp"

#ifdef _WIN32
  #include <windows.h>
  #include <iphlpapi.h>
  #include <rpc.h>
  #pragma comment(lib, "iphlpapi.lib")
  #pragma comment(lib, "rpcrt4.lib")
#elif __APPLE__
  #include <ifaddrs.h>
  #include <net/if_dl.h>
  #include <uuid/uuid.h>
#elif __linux__
  #include <sys/ioctl.h>
  #include <net/if.h>
  #include <unistd.h>
  #include <uuid/uuid.h>
  #include <net/if_types.h>
#endif
#include "Authentication.h"

#include <netdb.h>
#include <thread>

#include "Logger.h"

namespace comet
  {
    std::string getMacAddress()
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

    std::string getUuid()
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

    std::string generateMachineId(const std::string &mac, const std::string &uuid)
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

    bool Authentication::CheckVerificationInformation()
      {
        std::string requestUrl = "https://license.cometstrategy.com/cloudService/getApiInformation.php";
        std::list<std::string> cHeaders;

        // Retrieve settings
        std::string userEmail = email;
        std::string emailedVerificationCode = code;

        nlohmann::json jsonBuilder;
        jsonBuilder["email"] = email;
        jsonBuilder["emailedVerificationCode"] = code;


        // Add the obove headers to cheaders
        cHeaders.push_back("Content-type: application/json");
        cHeaders.push_back("X-Email: " + email);
        cHeaders.push_back("X-Code: " + code);
        auto response = Curl::post(requestUrl, jsonBuilder.dump(), cHeaders);

        if (response.isError()) {
          if (response.type == Curl::ResponseType::Error) {
            Logger::log(std::string("Licence Error: ") + std::to_string(response.status) + " - " + response.body,
                        LoggerLevel::CRITICAL);
          } else if (response.type == Curl::ResponseType::CurlError) {
            Logger::log(std::string("Licence Curl Error: ") + response.curlError + " at " + requestUrl,
                        LoggerLevel::CRITICAL);
          } else {
            Logger::log(std::string("Licence Undefined Error at ") + requestUrl, LoggerLevel::CRITICAL);
          }
          return false;
        }

        // get json response from body and read the isValid field
        nlohmann::json jsonResponse;
        try {
          jsonResponse = nlohmann::json::parse(response.body);
        } catch (const nlohmann::json::parse_error &e) {
          Logger::log("JSON Parse Error: " + std::string(e.what()), LoggerLevel::CRITICAL);
          return false;
        }
        if (!jsonResponse.contains("isValid") || !jsonResponse["isValid"].is_boolean()) {
          Logger::log("Invalid response format: 'isValid' field not found or not a boolean", LoggerLevel::CRITICAL);
          return false;
        }
        if (!jsonResponse["isValid"]) {
          Logger::log("Validation failed " + response.body, LoggerLevel::CRITICAL);
          return false;
        }
        return jsonResponse["isValid"].get<bool>();
      }


    size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
      {
        auto *str = static_cast<std::string *>(userp);
        str->append(static_cast<char *>(contents), size * nmemb);
        return size * nmemb;
      }

    std::string Authentication::getPrivateIPAddress()
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

    std::string Authentication::getPublicIPAddressFromWeb()
      {
        CURL *curl;
        CURLcode res;
        std::string publicIP = "";

        curl_global_init(CURL_GLOBAL_DEFAULT); // Global initialization

        curl = curl_easy_init();
        if (!curl) {
          std::cerr << "Failed to initialize CURL" << std::endl;
          curl_global_cleanup();
          return publicIP;
        }

        curl_easy_setopt(curl, CURLOPT_URL, "http://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &publicIP);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
          std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup(); // Global cleanup

        return publicIP;
      }

    std::string Authentication::getMachineId()
      {
        std::string mac = getMacAddress();
        if (mac.empty()) {
          std::cerr << "Failed to retrieve MAC address" << std::endl;
          return "";
        }

        std::string uuid = getUuid();
        if (uuid.empty()) {
          std::cerr << "Failed to generate UUID" << std::endl;
          return "";
        }
        return mac;
        /*
        uuid = "1234";

        machineId = generateMachineId(mac, uuid);
        return machineId;
        /**/
      }


    void Authentication::set_total_cores(int total_cores)
      {
        totalCores = total_cores;
      }

    void Authentication::set_unused_cores(int unused_cores)
      {
        unusedCores = unused_cores;
      }

    void Authentication::set_manager_ip_address(const std::string &manager_ip_address)
      {
        managerIpAddress = manager_ip_address;
      }

    Authentication::Authentication()
      {
        machineId = getMachineId();
        // Check Database for existing machine ID
        if (machineId.empty()) {
          Logger::log("Failed to generate machine ID", LoggerLevel::CRITICAL);
          throw std::runtime_error("Failed to generate machine ID");
        }

        isAuthenticated = false;
      }

    bool Authentication::valid(std::string newemail, std::string newcode, std::string newmachineId)
      {
        isAuthenticated = false;
        email = newemail; // Store email
        code = newcode; // Store code
        machineId = getMachineId(); // Store machine URL
        /* , int newTotalCores ,
                               int newUnusedCores, std::string newManagerIpAddress
        totalCores = newTotalCores;
        unusedCores = newUnusedCores;
        managerIpAddress = newManagerIpAddress/**/;
        if (code.size() < 5 || email.size() < 10 || machineId.size() < 10) {
          Logger::log("Invalid authentication parameters", LoggerLevel::CRITICAL);
          return false;
        }
        auto valid = CheckVerificationInformation();
        if (!valid) {
          return false;
        }
        // Save in the database
        isAuthenticated = true;
        return true;
      }

    std::string Authentication::HtmlAuthenticationForm()
      {
        std::string ip = getPublicIPAddressFromWeb();

        std::string html = "<p></p>  <h1>Authentication Settings</h1>";
        auto isValid = valid(email, code, machineId);
        if (isValid) {
          html += "<p>✅ Valid authentication settings</p>";
        } else {
          html += "<p>❌ Invalid authentication settings, update the email and code below.</p>";
        }


        html += "<form method=\"post\" action=\"/authenticate\">"
            "<table>"
            "<tr>"
            "<td><label for=\"email\">Email:</label></td>"
            "<td><input type=\"text\" id=\"email\" name=\"email\" value=\"" + email + "\"></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"code\">Code:</label></td>"
            "<td><input type=\"text\" id=\"code\" name=\"code\" value=\"" + code + "\"></td>" "</tr>"
            "<tr>"
            "<td><label for=\"ip\">IP Address:</label></td>"
            "<td><input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + ip +
            "\" readonly style=\"border: none; background-color: #f0f0f0;\"></td>"
            "</tr>""<tr>"
            "<td><label for=\"machineId\">Machine ID:</label></td>"
            "<td><input type=\"hidden\" id=\"machineId\" name=\"machineId\" value=\"" + machineId + "\">"
            "<p style=\"border: none; background-color: #f0f0f0;\">" + machineId + "</p></td>"
            "</tr>"
            "<td><input type=\"submit\" value=\"Update Authentication Settings\" class=\"ui primary button\"></td>"
            "<td></td>"
            "</tr>"
            "</table>"
            "</form>";

        if (isValid) {
          totalCores = std::thread::hardware_concurrency();
          // Example value, replace with actual logic to get total cores
          unusedCores = std::max(0, unusedCores); // Example value, replace with actual logic to get reserved cores

          html += "<p></p> <h1>Machine Settings</h1>"
              "<form method=\"post\" action=\"/configuration\">"
              "<table>"
              "<tr>"
              "<td><label for=\"totalCores\">Total Cores:</label></td>"
              "<td><input type=\"text\" id=\"totalCores\" name=\"totalCores\" value=\"" + std::to_string(totalCores) +
              "\" readonly style=\"border: none; background-color: #f0f0f0;\"></td>"
              "</tr>"
              "<tr>"
              "<td><label for=\"reservedCores\">Unused Cores:</label></td>"
              "<td><input type=\"number\" id=\"reservedCores\" name=\"unusedCores\" value=\"" +
              std::to_string(unusedCores) + "\"></td>"
              "</tr>"
              "<tr>"
              "<td><label for=\"servantManager\">Servant Manager Name/IP Address: <br>"
              "(Leave empty of this is the manager)</label></td>"
              "<td><input type=\"text\" id=\"servantManager\" name=\"servantManager\" value=\"" + managerIpAddress +
              "\"  style=\"border: none; background-color: #f0f0f0;\"></td>"
              "</tr>"
              "<tr>"
              "<td><input type=\"submit\" value=\"Update Machine Settings\" class=\"ui primary button\"></td>"
              "<td></td>"
              "</tr>"
              "</table>"
              "</form>";
        }

        return html;
      }
  } // comet
