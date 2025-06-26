//
// Created by Brett King on 26/6/2025.
//


#include <thread>

#include "Authentication.h"
#include "Curl.hpp"
#include "utilities.h"
#include "Servant.h"

#include "drogon/LocalHostFilter.h"

namespace comet
  {
    Servant::Servant()
      {
        totalCores = std::thread::hardware_concurrency();
        unusedCores = 0;
        activeCores = 0;
        managerIpAddress = ""; // Default empty manager IP address
      }


    void Servant::set_total_cores(int total_cores)
      {
        totalCores = total_cores;
      }

    void Servant::set_unused_cores(int unused_cores)
      {
        unusedCores = unused_cores;
      }

    void Servant::set_manager_ip_address(const std::string &manager_ip_address)
      {
        managerIpAddress = manager_ip_address;
      }

    std::string Servant::HtmlAuthenticationForm()
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

    void Servant::setAuthentication(Authentication *authState)
      {
        auth = authState;
      }

    bool Servant::routineStatusUpdates(const Authentication &auth) const
      {
        auto updateHostIPName = managerIpAddress;
        if (updateHostIPName.empty()) { updateHostIPName = "localhost"; }
        // Post to the manager IP address
        auto url = "http://" + updateHostIPName + ":7777/servant/status";
        nlohmann::json jsonData;
        jsonData["totalCores"] = totalCores;
        jsonData["unusedCores"] = unusedCores;
        jsonData["activeCores"] = activeCores;
        jsonData["managerIpAddress"] = managerIpAddress;
        jsonData["nameIpAddress"] = getPrivateIPAddress();
        jsonData["ServantVersion"] = "V 2025.06.26";
        jsonData["email"] = auth.getEmail();
        jsonData["code"] = auth.getCode();
        jsonData["ListeningPort"] = 7777; // Replace with actual port if needed
        jsonData["projectId"] = 1; // Replace with actual project ID if needed
        auto response = Curl::postJson(url, jsonData);
        if (response.isError()) {
          Logger::log("Failed to update Servant status on manager: " + response.curlError, LoggerLevel::CRITICAL);
          return false;
        } else {
          Logger::log("Servant status updated successfully on manager: " + updateHostIPName, LoggerLevel::INFO);
        }
        return true;
      }

    void Servant::startRoutineStatusUpdates(const Authentication &auth)
      {
        std::thread([this, auth]()
          {
            while (true) {
              auto bUpdated = routineStatusUpdates(auth);
              if (bUpdated)
                std::this_thread::sleep_for(std::chrono::hours(24)); // Update every 24 hours
              else
                //std::this_thread::sleep_for(std::chrono::minutes(5)); // Retry every 5 minutes on failure
                std::this_thread::sleep_for(std::chrono::seconds(10)); // Retry every 5 minutes on failure
            }
          }).detach(); // Detach the thread to run independently
      }
  } // comet
