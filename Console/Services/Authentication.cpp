//
// Created by Brett King on 17/6/2025.
//
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

#include <string>
#include <curl/curl.h>

#include "Authentication.h"

#include <netdb.h>

#include "Logger.h"

namespace comet
  {
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <openssl/sha.h>

    std::string getMacAddress()
      {
#ifdef __APPLE__
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
#define IFT_ETHER 0x6 // Ethernet constant
#endif
          if (sdl->sdl_type == IFT_ETHER) {
            // Ensure this constant is defined
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
        const char* iface = "eth0"; // Default for Linux; adjust as needed
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
        uuid_t uuid;
        uuid_generate(uuid);
        char uuidStr[37];
        uuid_unparse_lower(uuid, uuidStr);
        return std::string(uuidStr);
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

        machineId = generateMachineId(mac, uuid);
        return machineId;
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

    bool Authentication::validate(std::string newemail, std::string newcode, std::string newmachineId)
      {
        isAuthenticated = false;
        email = newemail; // Store email
        code = newcode; // Store code
        machineId = newmachineId; // Store machine URL
        if (code.size() < 5 || email.size() < 10 || machineId.size() < 10) {
          Logger::log("Invalid authentication parameters", LoggerLevel::CRITICAL);
          return false;
        }
        // Save in the database
        isAuthenticated = true;
        return true;
      }

    std::string Authentication::HtmlAuthenticationForm()
      {
        std::string ip = getPublicIPAddressFromWeb();

        std::string html = "<p></p>";
            "<h1>Authentication</h1>"
            "<p>Enter your email, code, and machine ID to authenticate.</p>"
            "<p>Your public IP address is: <strong>" + ip + "</strong></p>";
        if (validate(email, code, machineId)) {
          html += "<h2 class=\"success\">Authentication Valid</h2>";
        } else {
          html += "<h2 class=\"error\">Authentication Invalid</h2>";
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
            "</tr>"
            "<tr>"
            "<td><label for=\"machineId\">Machine ID:</label></td>"
            "<td><input type=\"text\" id=\"machineId\" name=\"machineId\" value=\"" + machineId +
            "\" readonly style=\"border: none; background-color: #f0f0f0;\"></td>"
            "</tr>" "<tr>"
            "<td><input type=\"submit\" value=\"Update\" class=\"ui primary button\"></td>"
            "<td></td>"
            "</tr>"
            "</table>"
            "</form>";

        return html;
      }
  } // comet
