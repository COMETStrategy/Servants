//
// Created by Brett King on 7/6/2025.
//


#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>


#include "Job.h"

#include "Curl.hpp"
#include "Logger.h"
#include "Encoding.h"
#include "Database.h"
#include "utilities.h"
#include "WebServices.h"
#include "drogon/orm/BaseBuilder.h"

namespace comet
  {
    Job::Job(const drogon::HttpRequestPtr &request, JobStatus newStatus, Database &db)
      {
        status = newStatus; // Set initial job status to Queued

        /* Sample json data
          {
            "type": "base64",
            "InputFileName": "S_Input 20250615 194550.txt",
            "TimeStamp": "",
            "timeout": "30m",
            "attributes": {
              "CaseNumber": "20.000009",
              "Title": "Minera Finca Ltda - Publish Data",
              "CaseName": "Base Case M20",
              "CreatorMachine": "brettking",
              "EngineVersion": "Comet",
              "EngineDirectory64": "L1ZvbHVtZXMvQ2FzZVNlbnNpdGl2ZURpc2svRGV2ZWxvcG1lbnQvRW5naW5lL091dHB1dC9SZWxlYXNlL29zeDY0Lw==",
              "BasePhaseDirectory64": "L1ZvbHVtZXMvQ2FzZVNlbnNpdGl2ZURpc2svRGV2ZWxvcG1lbnQvVXNlckludGVyZmFjZS9QaGFzZXMv",
              "WorkingDirectory64": "L1ZvbHVtZXMvQ2FzZVNlbnNpdGl2ZURpc2svRGV2ZWxvcG1lbnQvVXNlckludGVyZmFjZS9TY2hlZHVsZXMvTWluZXJhIEZpbmNhIEx0ZGEgLSBQdWJsaXNoIERhdGEvMjAuMDAwMDA1Lw==",
              "CaseBody64": "MjAJMjAJMC4wNQktNQlUUlVFCTAJMAkwCTA="
            }
      
         */
        try {
          auto json = request->getJsonObject();
          if (!json) {
            throw std::invalid_argument("Invalid JSON payload");
          }

          if (request->getHeader("X-Email").empty()) {
            throw std::invalid_argument("Email header is missing");
          }
          if (request->getHeader("X-Code").empty()) {
            throw std::invalid_argument("Code header is missing");
          }

          if (!json->isObject()) {
            throw std::invalid_argument("JSON root is not an object");
          }

          if (!json->isMember("InputFileName") || !(*json)["InputFileName"].isString()) {
            throw std::invalid_argument("Missing or invalid 'InputFileName' in JSON");
          }
          if (!(*json).isMember("TimeStamp") || !(*json)["TimeStamp"].isString()) {
            throw std::invalid_argument("Missing or invalid 'TimeStamp' in JSON");
          }
          // Additional checks...


          email = request->getHeader("X-Email");
          code = request->getHeader("X-Code");
          inputFileName = (*json)["InputFileName"].asString();
          timeStamp = (*json)["TimeStamp"].asString();
          timeout = (*json)["timeout"].asString();
          type = (*json)["type"].asString();
          caseNumber = (*json)["attributes"]["CaseNumber"].asString();
          title = (*json)["attributes"]["Title"].asString();
          caseName = (*json)["attributes"]["CaseName"].asString();
          creatorMachine = (*json)["attributes"]["CreatorMachine"].asString();

          // Decode Base64 and apply UTF-8 encoding
          engineVersion = (*json)["attributes"]["EngineVersion"].asString();
          engineDirectory = comet::Encoding::decode((*json)["attributes"]["EngineDirectory64"].asString());
          basePhaseDirectory = comet::Encoding::decode((*json)["attributes"]["BasePhaseDirectory64"].asString());
          workingDirectory = comet::Encoding::decode((*json)["attributes"]["WorkingDirectory64"].asString());
          caseBody = comet::Encoding::decode((*json)["attributes"]["CaseBody64"].asString());


          // Additional information
          creatorName = "Brett King"; // Replace with actual creator name
          creatorXEmail = request->getHeader("X-Email");
          creatorXCode = request->getHeader("X-Code");
          peopleRefValue = 456; // Replace with actual people reference value
          projectRefValue = 123; // Replace with actual project reference value
        } catch (const std::invalid_argument &e) {
          COMETLOG(e.what(), comet::INFO); // COMETLOG the error message
          status = JobStatus::Failed; // Set job status to Failed
          return;
        }

        updateAllInLocalDatabase(db);
      }

    std::string Job::getFullReplaceQueryString() const
      {
        auto Id = comet::generateTimestamp(); // Member function to generate timestamp
        auto query = std::string("") + "REPLACE INTO jobs (GroupName, CaseNumber, projectId, LastUpdate, "
                     "CaseName, ID, CreatorName, CreatorXEmail, CreatorXCode, CreatorMachine, peopleId, "
                     "InputFileName, EngineVersion, EngineDirectory, PhaseFileLocation, WorkingDirectory, "
                     "Status, RunProgress, Servant, Ranking, Life, IterationsComplete, RunTimeMin, CaseBody) "
                     "VALUES ('" + title + "', " + caseNumber + ", " + std::to_string(projectRefValue) +
                     ", datetime('now'), "
                     "'" + caseName + "', '" + Id + "', '" + creatorName + "', '" + creatorXEmail +
                     "', '" + creatorXCode + "', '" + creatorMachine + "', " + std::to_string(peopleRefValue) +
                     ", "
                     "'" + inputFileName + "', '" + engineVersion + "', '" + engineDirectory + "', '" +
                     basePhaseDirectory + "', '" + workingDirectory + "', "
                     + std::to_string(status) + ", '', NULL, NULL, NULL, NULL, NULL, '" + caseBody + "');";
        return query;
      }

    bool Job::createNewJobsTable(Database &db)
      {
        std::string query = R"(CREATE TABLE IF NOT EXISTS jobs (
                                LastUpdate DATETIME NOT NULL
                              , GroupName VARCHAR(1000) NOT NULL
                              , CaseNumber TEXT NOT NULL
                              , Status TINYINT NOT NULL
                              , CaseName VARCHAR(1000) NOT NULL
                              , Servant VARCHAR(100)
                              , IterationsComplete INT
                              , Ranking DOUBLE
                              , Life DOUBLE
                              , RunTimeMin DOUBLE
                              , RunProgress VARCHAR(2000) NOT NULL
                              , CreatorName VARCHAR(100) NOT NULL
                              , CreatorMachine VARCHAR(100) NOT NULL
                              , CreatorXEmail VARCHAR(100) NOT NULL
                              , CreatorXCode VARCHAR(100) NOT NULL
                              , peopleId INT NOT NULL
                              , projectId INT NOT NULL
                              , InputFileName VARCHAR(100) NOT NULL
                              , EngineVersion TEXT NOT NULL
                              , EngineDirectory VARCHAR(1000) NOT NULL
                              , PhaseFileLocation TEXT
                              , WorkingDirectory VARCHAR(1000) NOT NULL
                              , ProcessId VARCHAR(100)
                              , CaseBody TEXT NOT NULL
                              , UNIQUE(GroupName, CaseNumber, projectId)
                            );

                        
                        CREATE INDEX IF NOT EXISTS idx_ProcessId ON jobs (ProcessId);
                        CREATE INDEX IF NOT EXISTS idx_GroupNameOnly ON jobs (GroupName);
                        CREATE INDEX IF NOT EXISTS idx_Node ON jobs (Servant);
                        CREATE INDEX IF NOT EXISTS idx_projectId ON jobs (projectId);
                        CREATE INDEX IF NOT EXISTS idx_peopleId ON jobs (peopleId);
                        CREATE INDEX IF NOT EXISTS idx_UserCase ON jobs (CaseNumber);
                        )";

        db.createTableIfNotExists("jobs", query);
        return true;
      }

    std::string Job::jobSummaryHtmlReport(Database &db, std::string &sort, std::string &filter)
      {
        std::string html = "";


        // Save original sort and order
        if (sort.empty()) sort = "date";
        if (filter.empty()) filter = "all";
        auto rawSort = sort;
        auto rawFilter = filter;
        std::transform(sort.begin(), sort.end(), sort.begin(), ::tolower);
        std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

        std::string sortDescAsc = "DESC";
        if (sort != rawSort) sortDescAsc = "ASC";

        // Generate the base URL
        std::string baseUrl = "/";

        // Generate links for filters
        std::string filterLinks = "Filter by: ";
        std::vector<std::string> filters = {"All", "Queued", "Active", "Failed", "Complete"};
        std::string separator = "";
        for (const auto &aFilter: filters) {
          auto f = aFilter;
          std::transform(f.begin(), f.end(), f.begin(), ::tolower);
          filterLinks += separator;
          if (f == filter) {
            filterLinks += "<a class='highlight' href='" + baseUrl + "?sort=" + sort + "&filter=" + f + "'>" + aFilter + "</a>";
          } else {
            filterLinks += "<a href='" + baseUrl + "?sort=" + sort + "&filter=" + f + "'>" + aFilter + "</a>";
          }
          
          separator = " ";
        }

        // Generate links for sorting options
        std::string sortLinks = "Order by: ";
        std::vector<std::string> sorts = {"Status", "Date", "NPV", "Case", "Creator", "Servant"};
        separator = "";
        for (const auto &s: sorts) {
          std::string sortCaseAdjusted = s;
          std::string sortLowerCase = s;
          std::transform(sortLowerCase.begin(), sortLowerCase.end(), sortLowerCase.begin(), ::tolower);

          sortLinks += separator;
          sortLinks += "<a href='" + baseUrl;
          sortLinks += "?filter=" + filter;

          if (sortDescAsc == "DESC") {
            std::transform(sortCaseAdjusted.begin(), sortCaseAdjusted.end(), sortCaseAdjusted.begin(), ::toupper);
          } else {
            std::transform(sortCaseAdjusted.begin(), sortCaseAdjusted.end(), sortCaseAdjusted.begin(), ::tolower);
          }
          sortLinks += "&sort=" + sortCaseAdjusted + "' ";

          if (sortLowerCase == sort) {
            sortLinks += " class='highlight'";
            sortLinks += ">" + s + " (" + sortDescAsc + ")";
          } else {
            sortLinks += ">" + s;
          }
          sortLinks += "</a>";

          separator = " ";
        }

        // Add filter and sort links to the HTML
        html += "<h4>" + filterLinks + "</h4>";
        html += "<h4>" + sortLinks + "</h4>";

        // Get the job summary from the database
        std::string selection = "SELECT * FROM jobs ";

        std::string whereClause = "";
        if (filter == "all") {
          whereClause = "";
        } else if (filter == "queued") {
          whereClause = " WHERE Status IN (0) ";
        } else if (filter == "active") {
          whereClause = " WHERE Status IN (1, 2) ";
        } else if (filter == "complete") {
          whereClause = " WHERE Status IN (3, 4) ";
        } else if (filter == "failed") {
          whereClause = " WHERE Status = 3 ";
        }

        std::string orderBy = " ORDER BY LastUpdate " + sortDescAsc + " ";
        if (sort == "status") {
          orderBy = " ORDER BY Status " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
        } else if (sort == "date") {
          orderBy = " ORDER BY LastUpdate " + sortDescAsc + " ";
        } else if (sort == "npv") {
          orderBy = " ORDER BY Ranking " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
        } else if (sort == "creator") {
          orderBy = " ORDER BY CreatorName " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
        } else if (sort == "servant") {
          orderBy = " ORDER BY servant " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
        } else if (sort == "c ase") {
          orderBy = " ORDER BY CaseNumber " + sortDescAsc + " ";
        }
        auto query = selection + whereClause + orderBy + " LIMIT 500;";
        auto results = db.getQueryResults(query);

        if (results.empty()) {
          html += "<p>No jobs found.</p>";
          return html;
        }

        // Generate the table
        html += "<table style='border: none; border-collapse: separate; border-spacing: 1px 0;'>";
        html += "<tr><th>Last Update</th><th>Group Name</th><th>Case Number</th><th>Servant</th><th>Status</th>"
            "<th>Progress</th><th>NPV</th><th>Life</th><th>Case Name</th><th>Creator Name</th><th>Creator Machine</th>"
            "<th>Creator Email</th><th>Input File Name</th><th>ProcessID</th></tr>";

        int rowIndex = 0;
        for (const auto &row: results) {
          std::string rowClass = (rowIndex % 2 == 0) ? "even" : "odd";
          auto aStatus = static_cast<JobStatus>(stoi(row.at("Status")));
          html += "<tr class='" + rowClass + "'>";
          html += "<td>" + row.at("LastUpdate") + "</td>";
          html += "<td>" + row.at("GroupName") + "</td>";
          html += "<td>" + row.at("CaseNumber") + "</td>";
          html += "<td>" + (row.at("Servant").empty() ? "" : row.at("Servant")) + "</td>";
          //html += "<td>" + Job::jobStatusDescription(aStatus) + " (" + std::to_string(int(aStatus)) + ")" + "</td>";
          html += "<td>" + Job::jobStatusDescription(aStatus) + "</td>";
          html += "<td>" + row.at("RunProgress") + "</td>";
          html += "<td>" + row.at("Ranking") + "</td>";
          html += "<td>" + row.at("Life") + "</td>";
          html += "<td>" + row.at("CaseName") + "</td>";
          html += "<td>" + row.at("CreatorName") + "</td>";
          html += "<td>" + row.at("CreatorMachine") + "</td>";
          html += "<td>" + row.at("CreatorXEmail") + "</td>";
          html += "<td>" + row.at("InputFileName") + "</td>";
          html += "<td>" + row.at("ProcessId") + "</td>";
          html += "</tr>";
          rowIndex++;
        }
        html += "</table>";

        return html;
      }

    std::string Job::description()
      {
        return jobStatusDescription(status) + " : " + title + " #" + caseNumber + " (" + caseName + ") ";
      }

    bool Job::startJob(Database &db, nlohmann::json &job)
      {
        auto servant = WebServices::getServant();
        COMETLOG("🏃Job:Starting job " + to_string(job["GroupName"]) + " " + to_string(job["CaseNumber"])
                 + " on this servant. ", comet::INFO);
        auto executableEngine = to_string(job["EngineVersion"]);
        auto workingDirectory = to_string(job["WorkingDirectory"]);
        auto inputFileName = to_string(job["InputFileName"]);
        // run executable;
        COMETLOG("Job: Executing job with Engine: " + executableEngine + " in directory: " + workingDirectory +
                 " with input file: " + inputFileName, comet::DEBUGGING);
        int processId = Job::runExecutable(executableEngine, workingDirectory, inputFileName);
        COMETLOG("Process ID: " + std::to_string(processId), comet::INFO);
        
        // Update job on manager
        nlohmann::json json;
        json["GroupName"] = job["GroupName"];
        json["CaseNumber"] = job["CaseNumber"];
        json["Status"] = int(JobStatus::Running);
        json["Servant"] = servant.getIpAddress();
        json["LastUpdate"] = "datetime('now')"; // This will be updated by the database
        json["RunProgress"] = "Job started on this servant.";
        json["ProcessId"] = std::to_string(processId);
        // Update the job in the manager
        if (servant.getIpAddress() == getPrivateIPAddress()) {
          auto res = Job::runningProcessUpdate(db, json);
          COMETLOG("Updated this job status:, updated " + std::to_string(res) + " record", comet::DEBUGGING);
        } else {
          auto url = "http://" + servant.getIpAddress() + ":" + std::to_string(servant.getPort()) + "/job/status/database/update";
          auto result = Curl::postJson(url, json);
          COMETLOG("Updated manager job status: " + result.body, comet::DEBUGGING);
        }


        return true;
      }

    int Job::runExecutable(const std::string &executableEngine, const std::string &workingDirectory, const std::string &inputFileName) {
        try {
          // Make directory if it doesnt exist
          if (!std::filesystem::exists(workingDirectory)) {
            COMETLOG("Creating working directory: " + workingDirectory, comet::DEBUGGING);
            std::filesystem::create_directories(workingDirectory);
          } else {
            COMETLOG("Working directory already exists: " + workingDirectory, comet::DEBUGGING);
          }
          // Change the current working directory
          std::filesystem::current_path(workingDirectory);

          // Fork a new process
          pid_t pid = fork();
          if (pid == -1) {
            COMETLOG("Failed to fork process", comet::CRITICAL);
            throw std::runtime_error("Failed to fork process");
          }

          if (pid == 0) {
            // Child process: Execute the command
            std::string executablePath = "./" + executableEngine;
            execl(executablePath.c_str(), executableEngine.c_str(), inputFileName.c_str(), nullptr);

            // If execl fails
            COMETLOG("Failed to execute: " + executablePath, comet::CRITICAL);
            _exit(EXIT_FAILURE);
          }

          // Parent process: Return the process ID
          return pid;
        } catch (const std::exception &e) {
          COMETLOG("Exception in runExecutable: " + std::string(e.what()), comet::CRITICAL);
          return -1; // Indicate failure
        }
    }

    bool Job::runningProcessUpdate(Database &db, nlohmann::json &json)
      {
                
        auto query = std::string("UPDATE jobs SET ")
                      + "  Status = " + to_string(json["Status"]) + " "
                      + ", Servant = " + to_string(json["Servant"]) + " "
                      + ", LastUpdate = datetime('now') "
                      + ", ProcessId = " + to_string(json["ProcessId"]) + " "
                      + ", RunProgress = " + to_string(json["RunProgress"]) + " "
                     "WHERE CaseNumber = " + to_string(json["CaseNumber"]) + " "
                     "AND GroupName = " + to_string(json["GroupName"]) + ";";
        auto rowsImpacted = db.updateQuery("Update Job Status", query, false);

        return rowsImpacted == 1;
      }

    void Job::startJobOnServant(Database &db, std::map<std::string, std::string> &job,
                                std::__wrap_iter<std::map<std::string, std::string> *> servant)
      {
        COMETLOG("Job: Start job " + job.at("GroupName") + " " + job.at("CaseNumber")
                 + " on servant: " + servant->at("ipAddress") + ":" + servant->at("port"), comet::INFO);
        //
        
        nlohmann::json jsonData;
        // loop through add all the job map of strings and add to jsonData;
        for (const auto [label, value]: job) {
          jsonData[label] = value;
        }
        
        if (servant->at("ipAddress") == getPrivateIPAddress()) {
          Job::startJob(db, jsonData);
        } else {

          auto url = "http://" + servant->at("ipAddress") + ":" + servant->at("port") + "/job/start";
          std::list<std::string> headers;
          Curl::postJson(url, jsonData, headers);
        }
      }

    std::string Job::getJobProcessId(const std::map<std::string, std::string> &jobMap)
      {
        std::vector<std::string> keys = {"ProcessId", "GroupName", "CaseNumber", "CaseName", "CreatorName", "CreatorMachine"};
        // Join all these firlds if they exist
        std::string result = "";
        for (const auto &key: keys) {
          auto it = jobMap.find(key);
          if (it != jobMap.end()) {
            if (!result.empty()) {
              result += " - ";
            }
            result += it->second;
          }
        }
        return result;
      }

    int Job::mockRunJobs(const Database &db)
      {
        auto query = "UPDATE Jobs set Status = '" + std::to_string(int(JobStatus::Running)) + "'" +
                     ", Servant = 'MockServant', Ranking = 123.457, Life = 45.67, IterationsComplete = 0, RunTimeMin = 12.34, RunProgress = 'Completed Iteration 0', LastUpdate = datetime('now') "
                     +
                     "WHERE  CaseNumber  = 20.000012 ;";
        auto rowsImpacted = db.updateQuery("Reset Running Jobs", query, false);

        auto queryQueued = "UPDATE Jobs set Status = " + std::to_string(int(JobStatus::Queued)) +
                           ", Servant = NULL, Ranking = NULL, Life = NULL, IterationsComplete = NULL, RunTimeMin = NULL, RunProgress = '', LastUpdate = datetime('now'), ProcessId='' "
                           +
                           "WHERE  CaseNumber  = 20.000011 or CaseNumber  = 20.000025 or CaseNumber  = 20.00025 ;";
        rowsImpacted += db.updateQuery("Reset Queued Jobs", queryQueued, false);
        if (rowsImpacted == 0) {
          COMETLOG("Failed to reset running jobs", comet::CRITICAL);
          return 0;
        }
        return rowsImpacted;
      }

    int Job::resetRunningJobs(Database &db)
      {
        auto query = "UPDATE Jobs set Status = " + std::to_string(int(JobStatus::Queued)) +
                     ", Servant = NULL, Ranking = NULL, Life = NULL, IterationsComplete = NULL, RunTimeMin = NULL, RunProgress = '', LastUpdate = datetime('now') "
                     +
                     "WHERE  Status  = " + std::to_string(int(JobStatus::Running)) + " ;";
        auto rowsImpacted = db.updateQuery("Reset Running Jobs", query, false);
        if (rowsImpacted == 0) {
          COMETLOG("No running jobs reset", comet::CRITICAL);
          return 0;
        }
        return rowsImpacted;
      }

    bool Job::updateAllInLocalDatabase(Database &db) const
      {
        auto query = getFullReplaceQueryString();

        COMETLOG("Executing query: " + query, LoggerLevel::DEBUGGING);
        auto result = db.executeQuery(query);
        if (!result) {
          COMETLOG("Failed to execute database query: " + query, LoggerLevel::CRITICAL);
          return false; // Or handle the error appropriately
        }

        return true;
      }


    std::string Job::getAllJobStatuses(Database &db, std::string &GroupName)
      {
        std::string result;
        std::string query = "Select * from jobs where GroupName = '" + GroupName + "' order by LastUpdate Limit 500;";
        auto results = db.getQueryResults(query);
        if (results.empty()) {
          result = "No jobs found.";
          return result;
        }

        result =
            R"(Group\tCase#\tCase Name\tIteration\tNPV\tLife\tProcessId\tStatus\tProgress\tRun Time\tUpdated\tServant\tCreator\tEngine\tWorking Directory\n)";

        for (const auto &row: results) {
          auto aStatus = static_cast<JobStatus>(stoi(row.at("Status")));
          std::string lookupkey = "";
          // Get the following Group\tCase#\tCase Name\tIteration\tNPV\tLife\tProcessId\tStatus\tProgress\tRun Time\tUpdated\tServant\tCreator\tEngine\tWorking Directory\n
          try {
            std::vector<std::string> keys = {
              "GroupName", "CaseNumber", "CaseName", "IterationsComplete", "Ranking",
              "Life", "ProcessId", "Status", "RunProgress", "RunTimeMin", "LastUpdate",
              "Servant", "CreatorName", "EngineVersion", "WorkingDirectory"
            };

            std::string separator = "";
            for (const auto &key: keys) {
              lookupkey = key; // Update lookupkey before accessing the row
              auto value = row.at(key);
              if (key == "Status") {
                value = Job::jobStatusDescription(aStatus) + " (" + std::to_string(int(aStatus)) + ")";
              }
              result += separator + value;
              separator = R"(\t)";
            }

            result += R"(\n)";

            result += "\n"; // Add a newline at the end of the row
          } catch (const std::exception &e) {
            COMETLOG(std::string("Error processing key: ") + lookupkey + " - " + e.what(), comet::CRITICAL);
            result += "invalid " + lookupkey + "\t"; // Add an invalid record for the failed key
          }
        }
        return result;
      }

    JobStatus Job::jobStatus() const
      {
        return status;
      }

    bool Job::validJobStatus() const
      {
        return status != JobStatus::Failed;
      }

    std::string Job::jobStatusDescription(const JobStatus aStatus)
      {
        // Convert JobStatus to string description
        switch (aStatus) {
          case Queued: return "Queued";
          case Allocated: return "Allocated";
          case Running: return "Running";
          case Failed: return "Failed";
          case Completed: return "Completed";
          default: return "Unknown Status";
        }
      }

    JobStatus Job::setJobStatus(const JobStatus newStatus)
      {
        status = newStatus;
        return status;
      }
  }
