//
// Created by Brett King on 17/6/2025.
//
#include <string>
#include <nlohmann/json.hpp>
#include "Curl.hpp"
#include "utilities.h"

#ifdef _WIN32
  #include <windows.h>
  #include <iphlpapi.h>
  #include <rpc.h>
  #pragma comment(lib, "iphlpapi.lib")
  #pragma comment(lib, "rpcrt4.lib")
#elif __APPLE__
  #include <ifaddrs.h>
  #include <netdb.h>
#elif __linux__
  #include <sys/ioctl.h>
  #include <net/if.h>
  #include <unistd.h>
  #include <uuid/uuid.h>
  #include <net/if_types.h>
  #include <netdb.h>
#endif
#include "Authentication.h"

#include <thread>

#include "Logger.h"

namespace comet
  {
    Authentication::Authentication()
      {
        isAuthenticated = false;
      }

    bool Authentication::CheckVerificationInformation(std::string email, std::string code)
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
            COMETLOG(std::string("Licence Error: ") + std::to_string(response.status) + " - " + response.body,
                        LoggerLevel::CRITICAL);
          } else if (response.type == Curl::ResponseType::CurlError) {
            COMETLOG(std::string("Licence Curl Error: ") + response.curlError + " at " + requestUrl,
                        LoggerLevel::CRITICAL);
          } else {
            COMETLOG(std::string("Licence Undefined Error at ") + requestUrl, LoggerLevel::CRITICAL);
          }
          return false;
        }

        // get json response from body and read the isValid field
        nlohmann::json jsonResponse;
        try {
          jsonResponse = nlohmann::json::parse(response.body);
        } catch (const nlohmann::json::parse_error &e) {
          COMETLOG("JSON Parse Error: " + std::string(e.what()), LoggerLevel::CRITICAL);
          return false;
        }
        if (!jsonResponse.contains("isValid") || !jsonResponse["isValid"].is_boolean()) {
          COMETLOG("Invalid response format: 'isValid' field not found or not a boolean", LoggerLevel::CRITICAL);
          return false;
        }
        if (!jsonResponse["isValid"]) {
          COMETLOG("Validation failed " + response.body, LoggerLevel::CRITICAL);
          return false;
        }
        return jsonResponse["isValid"].get<bool>();
      }


    bool Authentication::valid(std::string email, std::string code, std::string aIpAddress)
      {
        isAuthenticated = false;
        if (code.size() < 5 || email.size() < 10 || aIpAddress.size() < 2) {
          COMETLOG("Invalid authentication parameters.", LoggerLevel::CRITICAL);
          return false;
        }
        if (!CheckVerificationInformation(email, code)) return false;
        
        isAuthenticated = true;
        COMETLOG("✅ Valid authentication settings", LoggerLevel::INFO);
        return true;
      }

    
  } // comet
