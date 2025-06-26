//
// Created by Brett King on 26/6/2025.
//


#include <thread>
#include <string>


#include "Authentication.h"
#include "Curl.hpp"
#include "utilities.h"
#include "Servant.h"

#include "Database.h"
#include "drogon/LocalHostFilter.h"

namespace comet
  {
    Servant::Servant()
      {
        totalCores = std::thread::hardware_concurrency();
        unusedCores = 0;
        activeCores = 0;
        port = 7777;
        projectId = 0;
        version = "V 2025.06.26";
        managerIpAddress = ""; // Default empty manager IP address
        ipAddress = getPrivateIPAddress();
      }


    void Servant::set_totalCores(int total_cores)
      {
        totalCores = total_cores;
      }

    void Servant::set_unusedCores(int unused_cores)
      {
        unusedCores = unused_cores;
      }

    void Servant::set_activeCores(int active_cores)
      {
        activeCores = active_cores;
      }

    void Servant::set_port(const int aPort)
      {
        port = aPort;
      }

    void Servant::set_projectId(const int aProjectId)
      {
        projectId = aProjectId;
      }

    std::string Servant::get_code()
      {
        return code;
      }

    std::string Servant::get_email()
      {
        return email;
      }

    void Servant::set_email(const std::string &aEmail)
      {
        email = aEmail;
      }

    void Servant::set_code(const std::string &aCode)
      {
        code = aCode;
      }

    void Servant::set_ipAddress(const std::string &aIpAddress)
      {
        ipAddress = aIpAddress;
      }

    void Servant::set_managerIpAddress(const std::string &aManagerIpAddress)
      {
        managerIpAddress = aManagerIpAddress;
      }

    void Servant::set_version(const std::string &aVersion)
      {
        version = aVersion;
      }

    void Servant::updateServantSettings(Database &db)
      {

        std::string query = "UPDATE servants SET projectId = 0, lastUpdateTime = DATETIME('now'), "
                            "version = '" + version + "', email = '" + email + "', code = '" + code + "', "
                            "port = " + std::to_string(port) + ", totalCores = " + std::to_string(totalCores) + ", "
                            "unusedCores = " + std::to_string(unusedCores) + ", activeCores = " + std::to_string(activeCores) + " "
                            "WHERE ipAddress = '" + ipAddress + "';";
        
        if (!db.updateQuery("Update Servants", query, false)) {
          // do an insert a new record
          query = "INSERT INTO servants (ipAddress, projectId, registrationTime, lastUpdateTime, "
                  "version, email, code, port, totalCores, unusedCores, activeCores, managerIpAddress) "
                  "VALUES ('" + ipAddress + "', 0, DATETIME('now'), DATETIME('now'), '1.0', '" + email + "', '" +
                  code + "', " + std::to_string(port) + ", " + std::to_string(totalCores) + ", " +
                  std::to_string(unusedCores) + ", " +
                  std::to_string(activeCores) + ",'" + managerIpAddress + "');";
          if (!db.insertRecord(query, false)) {
            comet::Logger::log("Failed to update Servants table with servant settings.", LoggerLevel::CRITICAL);
            return;
          }

          comet::Logger::log("Servant settings updated successfully in the database.", LoggerLevel::DEBUG);
        }
      }


    std::string Servant::HtmlAuthenticationSettingsForm(Authentication &auth)
      {
        std::string ip_public = getPublicIPAddressFromWeb();
        std::string ip_private = getPrivateIPAddress();
        std::string ip = ip_public + " (public), " + ip_private + " (private/local)";
        std::string html = "<p></p>  <h1>Authentication Settings</h1>";
        ipAddress = ip_private;
        auto isValid = auth.valid(email, code, ipAddress);
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
            "<td><label for=\"ipAddress\">Servant IP Address:</label></td>"
            "<td><input type=\"hidden\" id=\"ipAddress\" name=\"ipAddress\" value=\"" + ipAddress + "\">"
            "<div style=\"border: none; background-color: #f0f0f0;\">" + ipAddress + "</div></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"ip\">Reference IP Addresses:</label></td>"
            "<td><input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + ip +
            "\" readonly style=\"border: none; background-color: #f0f0f0;\" size=\"50\"></td>"
            "</tr>"
            "<td><input type=\"submit\" value=\"Update Authentication Settings\" class=\"ui primary button\"></td>"
            "<td></td>"
            "</tr>"
            "</table>"
            "</form>";


        return html;
      }

    std::string Servant::HtmlServantSettingsForm()
      {
        totalCores = std::thread::hardware_concurrency();
        // Example value, replace with actual logic to get total cores
        unusedCores = std::max(0, unusedCores); // Example value, replace with actual logic to get reserved cores

        std::string html = "";
        html += "<p></p> <h1>Servant Settings</h1>"
            "<form method=\"post\" action=\"/configuration\">"
            "<table>"
            "<tr>"
            "<td><label for=\"totalCores\">Total Cores:</label></td>"
            "<td style=\"text-align: right;\"><input type=\"text\" id=\"totalCores\" name=\"totalCores\" value=\"" +
            std::to_string(totalCores) +
            "\" readonly style=\"border: none; background-color: #f0f0f0;\"></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"reservedCores\">Unused Cores:</label></td>"
            "<td style=\"text-align: right;\"><input type=\"number\" id=\"reservedCores\" name=\"unusedCores\" value=\""
            +
            std::to_string(unusedCores) + "\"></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"activeCores\">Active Cores:</label></td>"
            "<td style=\"text-align: right;\"><input type=\"text\" id=\"activeCores\" name=\"activeCores\" value=\"" +
            std::to_string(activeCores) +
            "\"  style=\"border: none; background-color: #f0f0f0;\"></td>"
            "</tr>"
            "<tr>"
            "<td><label for=\"servantManager\">Servant Manager Name/IP Address: <br>"
            "(Leave empty of this is the manager)</label></td>"
            "<td style=\"text-align: right;\"><input type=\"text\" id=\"servantManager\" name=\"servantManager\" value=\""
            + managerIpAddress +
            "\"  style=\"border: none; background-color: #f0f0f0;\"></td>"
            "</tr>"
            "<tr>"
            "<td><input type=\"submit\" value=\"Update Machine Settings\" class=\"ui primary button\"></td>"
            "<td></td>"
            "</tr>"
            "</table>"
            "</form>";

        return html;
      }

    bool Servant::createNewServentsTable(Database &db)
      {
        std::string query = R"(CREATE TABLE IF NOT EXISTS `servants` (
                    `ipAddress` varchar(250) NOT NULL
                    , `projectId` int NOT NULL
                    , `registrationTime` datetime NOT NULL
                    , `lastUpdateTime` datetime NOT NULL
                    , `ServantVersion` varchar(30) NOT NULL
                    , `email` varchar(100) NOT NULL
                    , `code` varchar(100)NOT NULL
                    , `port` int NOT NULL
                    , `totalCores` int NOT NULL
                    , `unusedCores` int NOT NULL
                    , `activeCores` int NOT NULL
                    , `managerIpAddress` varchar(250) NOT NULL
                    , PRIMARY KEY(`ipAddress`, `projectId`)
                    );)";

        db.createTableIfNotExists("servants", query);
        return true;
      }

    void Servant::setAuthentication(Authentication *authState)
      {
        auth = authState;
      }

    bool Servant::routineStatusUpdates() const
      {
        auto updateHostIPName = managerIpAddress;
        if (updateHostIPName.empty()) { updateHostIPName = "localhost"; }
        // Post to the manager IP address
        auto url = "http://" + ipAddress + ":" + std::to_string(port) + "/servant/status";
        nlohmann::json jsonData;
        jsonData["ipAddress"] = getPrivateIPAddress();
        jsonData["totalCores"] = totalCores;
        jsonData["unusedCores"] = unusedCores;
        jsonData["activeCores"] = activeCores;
        jsonData["managerIpAddress"] = managerIpAddress;
        jsonData["version"] = version;
        jsonData["email"] = email;
        jsonData["code"] = code;
        jsonData["port"] = port; // Replace with actual port if needed
        jsonData["projectId"] = 0; // Replace with actual project ID if needed
        auto response = Curl::postJson(url, jsonData);
        if (response.isError()) {
          if (!response.body.empty())
            Logger::log("Failed to update Servant status on manager, body: " + response.body, LoggerLevel::CRITICAL);
          else
            Logger::log("Failed to update Servant status on manager, curl: " + response.curlError,
                        LoggerLevel::CRITICAL);
          return false;
        } else {
          Logger::log("Servant status updated successfully on manager: " + updateHostIPName + " ✅",
                      LoggerLevel::INFO);
        }
        return true;
      }

    void Servant::startRoutineStatusUpdates()
      {
        std::thread([this]()
          {
            while (true) {
              auto bUpdated = routineStatusUpdates();
              if (bUpdated)
                //std::this_thread::sleep_for(std::chrono::hours(2)); 
                std::this_thread::sleep_for(std::chrono::seconds(10));
              else
                std::this_thread::sleep_for(std::chrono::minutes(10));
            }
          }).detach(); // Detach the thread to run independently
      }


    void Servant::updateDatabase(Database &db)
      {
        if (!db.insertRecord(
          "INSERT INTO Servants (ipAddress, projectId, registrationTime, lastUpdateTime, version, email, code, port, totalCores, unusedCores, activeCores, managerIpAddress) "
          "VALUES ('" + ipAddress + "', '" + std::to_string(projectId) + "', DATETIME('now'), DATETIME('now'), '"  +
          version + "', '" + email + "', '" + code + "', '" + std::to_string(port) + "', '" +
          std::to_string(totalCores) + "', '" + std::to_string(unusedCores) + "', '" + std::to_string(activeCores) + "'"
          ", '" + managerIpAddress + "' )", false
        )) {
          // Just update the record but not the registration time
          if (!db.updateQuery("Update the Servant table",
                              "UPDATE Servants SET lastUpdateTime = DATETIME('now'), email = '" + email  + "' "
                              ", code = '" + code + "', port = '" + std::to_string(port)  + "' "
                              ", totalCores = '" + std::to_string(totalCores)  + "' "
                              ", unusedCores = '" + std::to_string(unusedCores)  + "' "
                              ", activeCores = '" + std::to_string(activeCores)  + "' "
                              ", version = '" + version + "' "
                              ", managerIpAddress = '" + managerIpAddress + "' "
                              "WHERE ipAddress = '" + ipAddress + "' and projectId = '" + std::to_string(
                                projectId) + "';")) {
            comet::Logger::log("Failed to insert or update Servants table with authentication information: ",
                               LoggerLevel::CRITICAL);
          }
        }
      }
  } // comet
