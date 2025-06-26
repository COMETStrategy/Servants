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
        managerIpAddress = newManagerIpAddress/**/
        ;
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
        Logger::log("Valid authentication settings ✅", LoggerLevel::INFO);
        return true;
      }

    std::string Authentication::HtmlAuthenticationForm()
      {
        std::string ip_public = getPublicIPAddressFromWeb();
        std::string ip_private = getPrivateIPAddress();
        std::string ip = ip_public + " (public), " +  ip_private + " (private/local)";
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
            "<td><input type=\"text\" id=\"email\" name=\"email\" value=\"" + email + "\" size=\"50\"></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"code\">Code:</label></td>"
            "<td><input type=\"text\" id=\"code\" name=\"code\" value=\"" + code + "\" size=\"50\"></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"ip\">IP Address:</label></td>"
            "<td><input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + ip +
            "\" readonly style=\"border: none; background-color: #f0f0f0;\" size=\"50\"></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"machineId\">Machine ID:</label></td>"
            "<td><input type=\"hidden\" id=\"machineId\" name=\"machineId\" value=\"" + machineId + "\">"
            "<p style=\"border: none; background-color: #f0f0f0;\">" + machineId + "</p></td>"
            "</tr>"
            "<td><input type=\"submit\" value=\"Update Authentication Settings\" class=\"ui primary button\"></td>"
            "<td></td>"
            "</tr>"
            "</table>"
            "</form>";


        return html;
      }
  } // comet
