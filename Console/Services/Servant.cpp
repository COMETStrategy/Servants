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
#include "Job.h"
#include "drogon/LocalHostFilter.h"

namespace comet
  {
    Servant *Servant::thisServant = nullptr;

    Servant::Servant()
      {
        totalCores = std::thread::hardware_concurrency();
        unusedCores = 0;
        activeCores = 0;
        port = 777;
        projectId = 0;
        version = "V 2025.06.26";
        priority = 1.0;
        managerIpAddress = ""; // Default empty manager IP address
        ipAddress = getMachineName();
        if (ipAddress.empty()) ipAddress = getPrivateIPAddress();
        alive = true;
        engineFolder = "";
        thisServant = this;
      }


    void Servant::setTotalCores(int total_cores)
      {
        totalCores = total_cores;
      }

    void Servant::setUnusedCores(int unused_cores)
      {
        unusedCores = unused_cores;
      }

    void Servant::setActiveCores(int active_cores)
      {
        activeCores = active_cores;
      }

    void Servant::setPort(const int aPort)
      {
        port = aPort;
      }

    void Servant::setProjectId(const int aProjectId)
      {
        projectId = aProjectId;
      }


    void Servant::setEmail(const std::string &aEmail)
      {
        email = aEmail;
      }

    void Servant::setCode(const std::string &aCode)
      {
        code = aCode;
      }

    void Servant::setIpAddress(const std::string &aIpAddress)
      {
        if (aIpAddress.find(':') == std::string::npos) {
          ipAddress = aIpAddress;
        } else {
          ipAddress = aIpAddress.substr(0, aIpAddress.find(':'));
        }
      }

    void Servant::setManagerIpAddress(const std::string &aManagerIpAddress)
      {
        managerIpAddress = aManagerIpAddress;
      }

    void Servant::setVersion(const std::string &aVersion)
      {
        version = aVersion;
      }

    void Servant::setPriority(const double aPriority)
      {
        priority = aPriority;
      }

    void Servant::setAlive(const int aAlive)
      {
        alive = aAlive;
      }

    void Servant::updateServantSettings(Database &db)
      {
        std::string query = "UPDATE servants SET projectId = 0, lastUpdateTime = DATETIME('now'), "
                            "version = '" + version + "', email = '" + email + "', code = '" + code + "', "
                            "port = " + std::to_string(port) + ", totalCores = " + std::to_string(totalCores) + ", "
                            "unusedCores = " + std::to_string(unusedCores) + ", activeCores = " + std::to_string(
                              activeCores) + ", " + "priority = " + std::to_string(priority) +
                            ", " + "alive = " + std::to_string(alive) + " "
                            "WHERE ipAddress = '" + ipAddress + "';";

        if (db.updateQuery("Update Servants", query, false) == 0) {
          // do an insert a new record
          query = "INSERT INTO servants (ipAddress, projectId, registrationTime, lastUpdateTime, "
                  "version, email, code, port, priority, totalCores, unusedCores, activeCores, managerIpAddress, engineFolder, alive) "
                  "VALUES ('" + ipAddress + "', 0, DATETIME('now'), DATETIME('now'), '1.0', '" + email + "', '" +
                  code + "', " + std::to_string(port) + "', " + std::to_string(priority) + ", " +
                  std::to_string(totalCores) + ", " +
                  std::to_string(unusedCores) + ", " +
                  std::to_string(activeCores) + ",'" + managerIpAddress + "','" + engineFolder + "'," +
                  std::to_string(alive) + ", );";
          if (!db.insertRecord(query, false)) {
            COMETLOG("Failed to update Servants table with servant settings.", LoggerLevel::CRITICAL);
            return;
          }

          COMETLOG("Servant settings updated successfully in the database.", LoggerLevel::DEBUGGING);
        }
      }

    std::vector<std::map<std::string, std::string> > Servant::getAvailableServants(Database &db)
      {
        auto availableServants = db.getQueryResults(
          "SELECT * FROM servants WHERE (totalCores-unusedCores-activeCores) > 0 and alive = true ORDER BY priority DESC;");
        return availableServants;
      }

    
    int Servant::deleteServants(const Database &db, const Json::Value &servants) {
        std::string whereClause  = "";
        if (servants.size() == 0) return 0;
        for (const auto &servant : servants) {
          if (servant.isMember("ipaddress")) {
            if (whereClause.empty()) {
              whereClause = " WHERE ";
            } else {
              whereClause += " OR ";
            }
            whereClause += "(ipAddress = '" + servant["ipaddress"].asString() + "' )";
           }
        }
        std::string query = "DELETE FROM servants" + whereClause + ";";
        int deletedServants = db.updateQuery("Delete Servant", query, false);
        if (deletedServants == 0) {
          COMETLOG("Failed to delete any servants", LoggerLevel::CRITICAL);
        } else {
          COMETLOG("Successfully deleted " + std::to_string(deletedServants) + " servants.", LoggerLevel::INFO);
        }
      return deletedServants;
 
    }
   
    std::vector<std::map<std::string, std::string> > Servant::getAllServants(Database &db)
      {
        auto availableServants = db.getQueryResults(
          "SELECT * FROM servants WHERE (totalCores-unusedCores-activeCores) > 0  ORDER BY priority DESC;");
        return availableServants;
      }

    void Servant::checkAllServantsAlive(Database &db)
      {
        const std::string thisIp = thisServant->getIpAddress();
        auto servants = getAllServants(db);
        for (auto &servant: servants) {
          auto isAlive = false;
          if (thisIp == servant.at("ipAddress")) {
            isAlive = true;
          } else {
            isAlive = check200Response(servant.at("ipAddress") + ":" + servant.at("port") + "/proofoflife");
          }
          auto aliveStatusText = (isAlive) ? "ALIVE" : "NOT ALIVE";
          if ((servant.at("alive") == "1" && !isAlive) || (servant.at("alive") == "0" && isAlive)) {
            COMETLOG("Servant " + servant.at("ipAddress") + " status changed: " + aliveStatusText, LoggerLevel::INFO);
            servant["alive"] = (isAlive) ? "1" : "0";
            auto updateQuery = "UPDATE servants SET alive = " + servant["alive"] +
                               ", lastUpdateTime = DATETIME('now') WHERE ipAddress = '" + servant.at("ipAddress") +
                               "';";
            auto result = db.updateQuery("Update Servant Alive Status", updateQuery, false);
          }
          COMETLOG("Servant " + servant.at("ipAddress") + " status: " + aliveStatusText, LoggerLevel::INFO);
        }
      }


    bool Servant::isManager()
      {
        return managerIpAddress != getManagerIpAddress();
      }

    std::string Servant::HtmlAuthenticationSettingsForm(Authentication &auth)
      {
        std::string ip_public = getPublicIPAddressFromWeb();
        std::string ip_private = getPrivateIPAddress();
        std::string ip = ip_public + " (public), " + ip_private + " (private/local)";
        std::string html = "";
        ipAddress = ip_private;
        auto isValid = auth.valid(email, code, ipAddress);
        if (isValid) {
          html += "<p>✅ Valid authentication settings</p>";
        } else {
          html += "<p>❌ Invalid authentication settings, update the email and code below.</p>";
        }

        std::string leftColumnStyles = " style='text-align: left;' ";

        html += "<form method=\"post\" action=\"/authenticate\">"
            "<table>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for=\"email\">Email:</label></td>"
            "<td><input type=\"text\" id=\"email\" name=\"email\" value=\"" + email + "\" ></td>"
            "</tr>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for=\"code\">Code:</label></td>"
            "<td><input type=\"text\" id=\"code\" name=\"code\" value=\"" + code + "\" ></td>"
            "</tr>"

            "<tr>"
            "<td " + leftColumnStyles + "><label for=\"ipAddress\">Servant IP Address:</label></td>"
            "<td><input type=\"hidden\" id=\"ipAddress\" name=\"ipAddress\" value=\"" + ipAddress + "\">"
            "<div class=number>" + ipAddress + "</div></td>"
            "</tr>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for=\"ip\">Reference IP Addresses:</label></td>"
            "<td><input type=\"text\" id=\"ip\" name=\"ip\" value=\"" + ip +
            "\" readonly></td>"
            "</tr>"
            "<td><input type=\"submit\" value=\"Update Authentication Settings\" class=\"ui primary button\"></td>"
            "<td></td>"
            "</tr>"
            "</table>"
            "</form>";


        return html;
      }

    std::string Servant::htmlServantSettingsForm()
      {
        totalCores = std::thread::hardware_concurrency();
        // Example value, replace with actual COMETLOGic to get total cores
        if (unusedCores < 0) unusedCores = 0; // Example value, replace with actual COMETLOGic to get reserved cores

        std::string html = "";
        std::string leftColumnStyles = " style='text-align: left;' ";
        html += "<form method='post' action='/configuration'>"
            "<table>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for='activeCores'>Active Cores:</label></td>"
            "<td style='text-align: right;'><input type='text' id='activeCores' name='activeCores' value='" +
            std::to_string(activeCores) + "'>" +
            "</td>"
            "</tr>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for='totalCores'>Total Cores:</label></td>"
            "<td style='text-align: right;'><input type='text' id='totalCores' name='totalCores' value='" +
            std::to_string(totalCores) +
            "' readonly ></td>"
            "</tr>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for='reservedCores'>Unused Cores:</label></td>"
            "<td style='text-align: right;'><input type='number' id='reservedCores' name='unusedCores' value='"
            +
            std::to_string(unusedCores) + "'></td>"
            "</tr>"
            "<tr>"
            "<td " + leftColumnStyles +
            "><label for='priority'>Priority (higher number for higher priority):</label></td>"
            "<td style='text-align: right;'><input type='text' id='priority' name='priority' value='" +
            std::to_string(priority) +
            "'  style='border: none; background-color: #f0f0f0;'></td>"
            "</tr>" "<tr>"
            "<td " + leftColumnStyles + "><label for='alive'>Enabled/Active (1 = enabled, 0 = not active):</label></td>"
            "<td style='text-align: right;'><input type='checkbox' id='alive' name='alive' value='1' " +
            (std::to_string(alive) == "1" ? "checked" : "") +
            " style='border: none; background-color: #f0f0f0;'></td>"
            "</tr>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for='managerIpAddress'>Servant Manager Name/IP Address: <br>"
            "(Leave empty of this is the manager)</label></td>"
            "<td style='text-align: right;'><input type='text' id='managerIpAddress' name='managerIpAddress' value='"
            + managerIpAddress +
            "'  style='border: none; background-color: #f0f0f0;'></td>"
            "</tr>"
            "<tr>"
            "<td " + leftColumnStyles + "><label for='engineFolder'>Engine folder:</label></td>"
            "<td style='text-align: right;'><input type='text' id='engineFolder' name='engineFolder' value='"
            + engineFolder +
            "'  style='border: none; background-color: #f0f0f0;'></td>"
            "</tr>"
            "<tr>"
            "<td><input type='submit' value='Update Machine Settings' class='ui primary button'></td>"
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
                    , priority double DEFAULT(1)
                    , `totalCores` int NOT NULL
                    , `unusedCores` int NOT NULL
                    , `activeCores` int NOT NULL
                    , `managerIpAddress` varchar(250) NOT NULL
                    , `engineFolder` varchar(1000) 
                    , alive bool not null
                    , PRIMARY KEY(`ipAddress`, `projectId`)
                    );)";

        db.createTableIfNotExists("servants", query);
        return true;
      }

    void Servant::setAuthentication(Authentication *authState)
      {
        auth = authState;
      }

    void Servant::setEngineFolder(const std::string &newFolder)
      {
        engineFolder = newFolder;
      }

    std::string Servant::getEngineFolder()
      {
        return engineFolder;
      }

    void Servant::stopSelectedProcesses(const Database &db, Json::Value &jobs)
      {
        // Build new json list of jobs to stop
        if (jobs.isNull() || jobs.empty()) {
          COMETLOG("No jobs to stop.", LoggerLevel::DEBUGGING);
          return;
        }

        std::string jobIds = "";
        for (const auto &job: jobs) {
          if (job.isMember("processid")) {
            jobIds += "'" + job["processid"].asString() + "',";
          }
        }
        if (!jobIds.empty()) {
          jobIds.pop_back(); // Remove the last comma
        } else {
          COMETLOG("No valid ProcessId found in jobs.", LoggerLevel::DEBUGGING);
          return;
        }

        std::string query = "SELECT Servant, GROUP_CONCAT(ProcessId, ',') AS ProcessIds FROM jobs "
                            " WHERE ProcessId IN (" + jobIds + ") GROUP BY Servant ORDER BY Servant, ProcessId;";

        auto results = db.getQueryResults(query);
        // Send stop requests to each servant using api /servant/stop_processes
        for (const auto &row: results) {
          std::string servantIp = row.at("Servant");
          std::string processIds = row.at("ProcessIds");
          if (servantIp  == getMachineName()) {
            // Stop processes locally
            Job::stopProcessesLocally(processIds);
          } else {
            // Send stop request to the servant
            nlohmann::json jsonData;
            jsonData["processIds"] = processIds;
            auto url = "http://" + servantIp + "/servant/stop_processes";
            auto response = Curl::postJson(url, jsonData);
            if (response.isError()) {
              COMETLOG("Failed to stop processes on servant " + servantIp + ": " + response.body,
                       LoggerLevel::CRITICAL);
            } else {
              COMETLOG("Successfully sent stop request to servant " + servantIp, LoggerLevel::DEBUGGING);
            }
          }
        }
      }

    void Servant::initialiseAllServantActiveCores(const Database &db)
      {
        // use the jobs table where the count of the JobStatus::running status in each servant to initialise the servant activeCores

        auto query = "UPDATE servants SET activeCores = IFNULL("
                     "(SELECT COUNT(*) FROM jobs "
                     "WHERE jobs.Servant = servants.ipAddress "
                     "AND status = " + std::to_string(int(JobStatus::Running)) + " OR status = " + std::to_string(
                       JobStatus::Allocated) + "), 0)"
                     ";";
        auto result = db.updateQuery("Initialise Active Cores", query, true);
        COMETLOG("Initialise Active Cores for "+std::to_string(result) + " servants.", LoggerLevel::INFO);
      }

    void Servant::decrementActiveProcessCount(Database &db, int decrementAmount)
      {
        db.updateQuery("Update Servant State",
                       "UPDATE servants SET activeCores = activeCores - " + std::to_string(decrementAmount) +
                       " WHERE ipAddress = '" + thisServant->ipAddress + "' ;");
      }

    std::string Servant::servantSummaryHtmlReport(Database &db)
      {
        std::string html = "";

        // Add filter and sort links to the HTML
        auto query = "SELECT * FROM Servants ORDER BY LastUpdateTime DESC LIMIT 500;";
        auto results = db.getQueryResults(query);

        if (results.empty()) {
          html += "<p>No servants found.</p>";
          return html;
        }

        // Generate the table
        html += "<table>";
        html +=
            "<tr>"
            "<th>" "<input type='checkbox' id='toggleAll' onchange='toggleLinks()' />" "</th>"
            "<th>Alive</th>"
            "<th>Last Update</th>"
            "<th>IP Address</th><th>Priority</th>"
            "<th>Total Cores</th><th>Unused Cores</th>"
            "<th>Active Cores</th>"
            "<th>Manager IP</th>"
            "<th>Engine Folder</th>"
            "</tr>";

        int rowIndex = 0;
        for (const auto &row: results) {
          std::string rowClass = (rowIndex % 2 == 0) ? "even" : "odd";
          std::string eFolder = "";
          if (row.find("engineFolder") != row.end()) {
            eFolder = std::string(row.at("engineFolder"));
          }
          html += "<tr class='" + rowClass + "'>";
          std::string checkbox = "<input type='checkbox' name='selectedjobs' unchecked "
                                 " data-ipAddress='" + row.at("ipAddress") + "'"
                                 ">";
          html += "<td>" + checkbox + "</td>";
          html += "<td>" + row.at("alive") + "</td>";
          html += "<td>" + row.at("lastUpdateTime") + "</td>";
          html += "<td>" + row.at("ipAddress") + "</td>";
          html += "<td>" + row.at("priority") + "</td>";
          html += "<td>" + row.at("totalCores") + "</td>";
          html += "<td>" + row.at("unusedCores") + "</td>";
          html += "<td>" + row.at("activeCores") + "</td>";
          html += "<td>" + row.at("managerIpAddress") + "</td>";
          html += "<td>" + eFolder + "</td>";
          html += "</tr>";
          rowIndex++;
        }
        html += "</table>";
        html += "<div id='linksContainer' style='display: none;'>";
        html += " <a href='#' id='deleteLink' class='highlight'  targetAddress='/servant/selected_delete'>Delete</a>";
        html += "</div>";

        html += "<p></p><h2>Options for Servants</h2>"
            "<span id=message ><button onclick=\"writeMessage(); location.href='/updateAliveServants'\" >Update Alive Servants</button>"
            "</span>"
            "<script>function writeMessage() {document.getElementById('message').textContent = 'Please wait, contacting Servants...';}</script>"
            "";

        return html;
      }

    bool Servant::routineStatusUpdates() const
      {
        // Post to the manager IP address
        auto url = "http://" + ipAddress + ":" + std::to_string(port) + "/servant/status";
        std::string servantLocation = "Local";
        if (!managerIpAddress.empty()) {
          url = "http://" + managerIpAddress + ":" + std::to_string(port) + "/servant/status";
          servantLocation = "Manager";
        }
        nlohmann::json jsonData;
        jsonData["ipAddress"] = getPrivateIPAddress();
        jsonData["totalCores"] = totalCores;
        jsonData["unusedCores"] = unusedCores;
        jsonData["activeCores"] = activeCores;
        jsonData["managerIpAddress"] = managerIpAddress;
        jsonData["engineFolder"] = engineFolder;
        jsonData["version"] = version;
        jsonData["email"] = email;
        jsonData["code"] = code;
        jsonData["port"] = port;
        jsonData["priority"] = priority;
        jsonData["alive"] = alive;
        jsonData["projectId"] = 0; // Replace with actual project ID if needed
        // Only update the local is it is not also the maanager
        COMETLOG("Servant is updating " + servantLocation + " status.", LoggerLevel::DEBUGGING);
        auto response = Curl::postJson(url, jsonData);
        if (response.isError()) {
          if (!response.body.empty())
            COMETLOG("Failed to update Servant status (" + servantLocation + "), body: " + response.body,
                   LoggerLevel::CRITICAL);
          else
            COMETLOG("Failed to update Servant status (" + servantLocation + "), curl: " + response.curlError,
                   LoggerLevel::CRITICAL);
          return false;
        } else {
          COMETLOG("Servant status updated successfully (" + servantLocation + "): " + ipAddress + " ✅",
                   LoggerLevel::DEBUGGING);
        }


        return true;
      }

    void Servant::startRoutineStatusUpdates()
      {
        // wait for 2 seconds
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::thread([this]()
          {
            while (true) {
              auto bUpdated = routineStatusUpdates();
              if (bUpdated)
                std::this_thread::sleep_for(std::chrono::hours(1));
                //std::this_thread::sleep_for(std::chrono::seconds(10));
              else
                std::this_thread::sleep_for(std::chrono::minutes(10));
            }
          }).detach(); // Detach the thread to run independently
      }


    void Servant::updateDatabase(Database &db)
      {
        if (!db.insertRecord(
          "INSERT INTO Servants (ipAddress, projectId, registrationTime, lastUpdateTime, version, email"
          ", code, port, totalCores, unusedCores, activeCores, managerIpAddress, engineFolder) "
          "VALUES ('" + ipAddress + "', '" + std::to_string(projectId) + "', DATETIME('now'), DATETIME('now'), '" +
          version + "', '" + email + "', '" + code + "', '" + std::to_string(port) + "', '" +
          std::to_string(totalCores) + "', '" + std::to_string(unusedCores) + "', '" + std::to_string(activeCores) + "'"
          ", '" + managerIpAddress + "' , '" + engineFolder + "' )", false
        )) {
          // Just update the record but not the registration time
          if (db.updateQuery("Update the Servant table",
                             "UPDATE Servants SET lastUpdateTime = DATETIME('now'), email = '" + email + "' "
                             ", code = '" + code + "', port = '" + std::to_string(port) + "', priority = '" +
                             std::to_string(priority) + "' "
                             ", totalCores = '" + std::to_string(totalCores) + "' "
                             ", unusedCores = '" + std::to_string(unusedCores) + "' "
                             ", activeCores = '" + std::to_string(activeCores) + "' "
                             ", version = '" + version + "' "
                             ", managerIpAddress = '" + managerIpAddress + "' "
                             ", engineFolder = '" + engineFolder + "' "
                             ", alive = " + std::to_string(alive) + " "
                             "WHERE ipAddress = '" + ipAddress + "' and projectId = '" + std::to_string(
                               projectId) + "';") == 0) {
            COMETLOG("Failed to insert or update Servants table with authentication information: ",
                     LoggerLevel::CRITICAL);
          }
        }
      }
  } // comet
