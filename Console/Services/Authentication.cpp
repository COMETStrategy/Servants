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

namespace comet {

    size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        auto *str = static_cast<std::string *>(userp);
        str->append(static_cast<char *>(contents), size * nmemb);
        return size * nmemb;
    }
    
    std::string Authentication::getPrivateIPAddress() {
        struct ifaddrs *ifaddr, *ifa;
        char host[NI_MAXHOST];
        std::string privateIP = "Not found";

        if (getifaddrs(&ifaddr) == -1) {
          perror("getifaddrs");
          return privateIP;
        }

        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
          if (ifa->ifa_addr == nullptr) continue;

          if (ifa->ifa_addr->sa_family == AF_INET) { // IPv4
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) == 0) {
              std::string interfaceName = ifa->ifa_name;
              if (interfaceName != "lo0") { // Exclude loopback interface
                privateIP = host;
                break;
              }
            }
          }
        }

        freeifaddrs(ifaddr);
        return privateIP;
    }

    std::string Authentication::getPublicIPAddressFromWeb() {
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

    
    

    inline bool Authentication::registerServant(std::string email, std::string code, std::string machineUrl)
      {  return true;
      }

    std::string Authentication::HtmlAuthenticationForm()
      {
        // Get ipaddress and machine name
        
        std::string ip = getPublicIPAddressFromWeb();
        return "<h2>Authentication</h2>"
                "<form method=\"post\" action=\"/authenticate\">"
                "<label for=\"email\">Email:</label><br>"
                "<input type=\"text\" id=\"email\" name=\"email\"><br>"
                "<label for=\"code\">Code:</label><br>"
                "<input type=\"text\" id=\"code\" name=\"code\"><br>"
                "<label for=\"ip\">IP Address:</label><br>"
                "<input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + ip + "\" readonly><br>"
                "<input type=\"submit\" value=\"Submit\">"
                "</form>";
      }
} // comet

