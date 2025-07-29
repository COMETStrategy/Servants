//
// Created by Brett King on 7/6/2025.
//


#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <fcntl.h>
#include <thread>


#include <csignal>
// #ifdef _WIN32
//   #include <windows.h>
// #else
//   #include <unistd.h>     // fork, execl
//   #include <fcntl.h>      // open
// #endif


#include "Job.h"
#include "../Utilities/Curl.hpp"
#include "../Utilities/Logger.h"
#include "../Utilities/Encoding.h"
#include "../Utilities/Database.h"
#include "../Utilities/utilities.h"
#include "WebServices.h"
#include "drogon/orm/BaseBuilder.h"

namespace comet
{
  Job::Job(const drogon::HttpRequestPtr& request, JobStatus newStatus, Database& db)
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
    try
    {
      auto json = request->getJsonObject();
      if (!json)
      {
        throw std::invalid_argument("Invalid JSON payload");
      }

      if (request->getHeader("X-Email").empty())
      {
        throw std::invalid_argument("Email header is missing");
      }
      if (request->getHeader("X-Code").empty())
      {
        throw std::invalid_argument("Code header is missing");
      }

      if (!json->isObject())
      {
        throw std::invalid_argument("JSON root is not an object");
      }

      if (!json->isMember("InputFileName") || !(*json)["InputFileName"].isString())
      {
        throw std::invalid_argument("Missing or invalid 'InputFileName' in JSON");
      }
      if (!(*json).isMember("TimeStamp") || !(*json)["TimeStamp"].isString())
      {
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
    }
    catch (const std::invalid_argument& e)
    {
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
      "CaseName, ProcessId, CreatorName, CreatorXEmail, CreatorXCode, CreatorMachine, peopleId, "
      "InputFileName, EngineVersion, EngineDirectory, PhaseFileLocation, WorkingDirectory, "
      "Status, RunProgress, Servant, Ranking, Life, IterationsComplete, RunTimeMin, CaseBody) "
      "VALUES ('" + title + "', '" + caseNumber + "', " + std::to_string(projectRefValue) +
      ", datetime('now'), "
      "'" + caseName + "', '" + Id + "', '" + creatorName + "', '" + creatorXEmail +
      "', '" + creatorXCode + "', '" + creatorMachine + "', " + std::to_string(peopleRefValue) +
      ", "
      "'" + inputFileName + "', '" + engineVersion + "', '" + engineDirectory + "', '" +
      basePhaseDirectory + "', '" + workingDirectory + "', "
      + std::to_string(status) + ", '', NULL, NULL, NULL, NULL, NULL, '" + caseBody + "');";
    return query;
  }

  bool Job::createNewJobsTable(Database& db)
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

  std::string Job::htmlGenerateFilterLinks(const std::string& baseUrl, const std::string& sort, std::string filter)
  {
    if (filter.empty()) filter = "all";
    auto rawFilter = filter;
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    std::string filterLinks = "Filter by: ";
    std::vector<std::string> filters = {"All", "Queued", "Active", "Failed", "Complete"};
    std::string separator = "";

    for (const auto& aFilter : filters)
    {
      auto f = aFilter;
      std::transform(f.begin(), f.end(), f.begin(), ::tolower);
      filterLinks += separator;
      if (f == filter)
      {
        filterLinks += "<a class='highlight' href='" + baseUrl + "?sort=" + sort + "&filter=" + f + "'>" + aFilter +
          "</a>";
      }
      else
      {
        filterLinks += "<a href='" + baseUrl + "?sort=" + sort + "&filter=" + f + "'>" + aFilter + "</a>";
      }
      separator = " ";
    }

    return filterLinks;
  }

  std::string Job::htmlGenerateSortLinks(const std::string& baseUrl, std::string& inputSort, std::string filter)
  {
    if (inputSort.empty()) inputSort = "date";
    auto inputSortLower = lower(inputSort);

    std::string sortDescAsc = (inputSort == inputSortLower) ? "DESC" : "ASC";

    std::string sortLinks = "Order by: ";
    std::vector<std::string> sorts = {"Status", "Date", "NPV", "Case", "Creator", "Servant"};
    std::string separator = "";

    for (const auto& s : sorts)
    {
      std::string sortCaseAdjusted = s;
      std::string sortLowerCase = lower(s);

      sortLinks += separator;
      sortLinks += "<a href='" + baseUrl;
      sortLinks += "?filter=" + filter;

      if (sortDescAsc == "DESC")
      {
        sortCaseAdjusted = upper(sortCaseAdjusted);
      }
      else
      {
        sortCaseAdjusted = lower(sortCaseAdjusted);;
      }
      sortLinks += "&sort=" + sortCaseAdjusted + "' ";

      if (sortLowerCase == inputSortLower)
      {
        sortLinks += " class='highlight'";
        sortLinks += ">" + s + " (" + sortDescAsc + ")";
      }
      else
      {
        sortLinks += ">" + s;
      }
      sortLinks += "</a>";

      separator = " ";
    }

    return sortLinks;
  }

  std::string Job::htmlJobSummaryReport(Database& db, std::string& sort, std::string& filter)
  {
    std::string html = "";


    // Get the job summary from the database
    std::string selection = "SELECT * FROM jobs ";

    std::string whereClause = "";
    if (filter == "all")
    {
      whereClause = "";
    }
    else if (filter == "queued")
    {
      whereClause = " WHERE Status IN (" + std::to_string(JobStatus::Initialised) + ", " + std::to_string(
        JobStatus::Queued) + ") ";
    }
    else if (filter == "active")
    {
      whereClause = " WHERE Status IN (" + std::to_string(JobStatus::Allocated) + ", " + std::to_string(
        JobStatus::Running) + ") ";
    }
    else if (filter == "complete")
    {
      whereClause = " WHERE Status IN (" + std::to_string(JobStatus::Failed) + ", " + std::to_string(
        JobStatus::Completed) + ") ";
    }
    else if (filter == "failed")
    {
      whereClause = " WHERE Status IN (" + std::to_string(JobStatus::Initialised) + ", " + std::to_string(
        JobStatus::Failed) + ") ";
    }
    std::string sortDescAsc = (sort[0] != std::tolower(sort[0])) ? "ASC" : "DESC";
    std::string sortOrder = comet::lower(sort);
    std::string orderBy = " ORDER BY LastUpdate " + sortDescAsc + " ";
    if (sortOrder == "status")
    {
      orderBy = " ORDER BY Status " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
    }
    else if (sortOrder == "date")
    {
      orderBy = " ORDER BY LastUpdate " + sortDescAsc + " ";
    }
    else if (sortOrder == "npv")
    {
      orderBy = " ORDER BY Ranking " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
    }
    else if (sortOrder == "creator")
    {
      orderBy = " ORDER BY CreatorName " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
    }
    else if (sortOrder == "servant")
    {
      orderBy = " ORDER BY servant " + sortDescAsc + ", LastUpdate " + sortDescAsc + " ";
    }
    else if (sortOrder == "case")
    {
      orderBy = " ORDER BY CAST(CaseNumber AS REAL) " + sortDescAsc + " ";
    }
    auto query = selection + whereClause + orderBy + " LIMIT 500;";
    auto results = db.getQueryResults(query);

    if (results.empty())
    {
      html += "<p>No jobs found.</p>";
      //return html;
    }
    else
    {
      // Generate the table
      html += "<table style='border: none; border-collapse: separate; border-spacing: 1px 0;'>";
      html += "<tr>"
        "<th>" "<input type='checkbox' id='toggleAll' onchange='toggleLinks()' />" "</th>"
        "<th>Last Update</th>"
        "<th>Group Name</th>"
        "<th class='centerAlign'>Case<br>Number</th>"
        "<th>Creator</th>"
        "<th class='centerAlign'>Status</th>"
        "<th class='centerAlign'>Progress</th>"
        "<th>Servant</th>"
        "<th class='rightAlign'>NPV</th>"
        "<th class='rightAlign'>Life</th>"
        "</tr>";
      int rowIndex = 0;
      for (const auto& row : results)
      {
        std::string rowClass = (rowIndex % 2 == 0) ? "even" : "odd";
        auto aStatus = static_cast<JobStatus>(stoi(row.at("Status")));
        rowClass += " Status" + Job::jobStatusDescription(aStatus);
        html += "<tr class='" + rowClass + "'>";
        // Add a checkbox for selection with the id CaseNumber and a custom attribute with the GroupName
        std::string checkbox = "<input type='checkbox' name='selectedjobs' unchecked "
          " data-casenumber='" + row.at("CaseNumber") + "'"
          " data-groupname='" + row.at("GroupName") + "'"
          " data-processid='" + row.at("ProcessId") + "'"
          " data-servant='" + row.at("Servant") + "'"
          ">";
        html += "<td>" + checkbox + "</td>";
        html += "<td>" + row.at("LastUpdate") + "</td>";
        html += "<td>" + row.at("GroupName") + "</td>";

        html += "<td class='centerAlign'><a href='#' title='Open Working Directory for input file " +
                 row.at("InputFileName") + "' onclick=\"openLocalFile('" +
                 jsEscape(row.at("WorkingDirectory")) + "')\">" + row.at("CaseNumber") + "</a></td>";

         html += "<td><span "
          "title='Machine: " + row.at("CreatorMachine") + ", Email: " + row.at("CreatorXEmail") + "'>"
          + row.at("CreatorName") + "</span></td>";
        auto title = (row.at("ProcessId").empty()) ? "" : " (#" + row.at("ProcessId") + ")";
        html += "<td class='centerAlign'>" + Job::jobStatusDescription(aStatus) + " " + title + "</td>";
        if (aStatus == JobStatus::Running || aStatus == JobStatus::Completed || aStatus == JobStatus::Failed)
        {
          auto progress = row.at("RunProgress");
          // truncate too long, more that 20 characters
          if (progress.length() > 20)
          {
            progress = progress.substr(0, 20) + "...";
          }
          html +=
            "<td class='centerAlign'><a href='#' title='Open Display File for more details' onclick=\"openLocalFile('"
            + jsEscape(row.at("WorkingDirectory")) + "S_Display.txt')\">" +
            progress +
            "</a></td>";
        }
        else
        {
          html += "<td>" + row.at("RunProgress") + "</td>";
        }
        html += "<td>" + (row.at("Servant").empty() ? "" : row.at("Servant")) + "</td>";


        if (!row.at("Ranking").empty())
        {
          double ranking = std::stod(row.at("Ranking"));
          std::ostringstream stream;
          stream << std::fixed << std::setprecision(2) << ranking;
          html +=
            "<td class='rightAlign'> <a href='#' title='Open Schedule HTML File for more details' onclick=\"openLocalFile('"
            + jsEscape(row.at("WorkingDirectory")) + "S_Schedule.html')\">" +
            stream.str() +
            "</a> </td>";
        }
        else
        {
          html += "<td></td>";
        }

        if (!row.at("Life").empty())
        {
          double life = std::stod(row.at("Life"));
          std::ostringstream stream;
          stream << std::fixed << std::setprecision(2) << life;
          html += "<td class='rightAlign'> " + stream.str() + " </td>";
        }
        else
        {
          html += "<td></td>";
        }

        html += "</tr>";
        rowIndex++;
      }
      html += "</table>"
        "<div id='linksContainer' style='display: none;'>"
        "<a href='#' id='stopLink' class='highlight' targetAddress='/jobs/selected_stop' >Stop</a>"
        " <a href='#' id='restartLink' class='highlight' targetAddress='/jobs/selected_restart' >Restart</a>"
        " <a href='#' id='deleteLink' class='highlight'  targetAddress='/jobs/selected_delete'>Delete</a>"
        "</div>"
        "";
    }
    // Add filter and sort links to the HTML
    std::string baseUrl = "/job/summary";
    html += "<p>" + htmlGenerateFilterLinks(baseUrl, sort, filter) + "<br>";
    html += "" + htmlGenerateSortLinks(baseUrl, sort, filter) + "</p>";


    return html;
  }

  std::string Job::description()
  {
    return jobStatusDescription(status) + " : " + title + " #" + caseNumber + " (" + caseName + ") ";
  }

  void Job::stopProcessesLocally(Database& db, const std::string& ProcessIds)
  {
    COMETLOG("Stopping processes with IDs: " + ProcessIds, LoggerLevel::INFO);
    auto processIds = split(ProcessIds, ',');
    for (const auto& pid : processIds)
    {
      bool stopped = false;
      if (pid.empty()) continue;
      COMETLOG("Stopping process ID: " + pid, LoggerLevel::DEBUGGING);
      try
      {
        int processId = std::stoi(pid);
        if (processId > 0)
        {
#ifdef _WIN32
          // Windows: Try native API first
          HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
          if (hProcess)
          {
            BOOL result = TerminateProcess(hProcess, 1);
            if (result)
            {
              stopped = true;
              COMETLOG("Successfully stopped process ID: " + pid, LoggerLevel::INFO);
            }
            else
            {
              COMETLOG("Failed to stop process ID: " + pid, LoggerLevel::CRITICAL);
            }
            CloseHandle(hProcess);
          }
          else
          {
            // If the API fails, fallback to system() with taskkill
            std::string command = "taskkill /PID " + std::to_string(processId) + " /F";
            int result = system(command.c_str());
            if (result == 0)
            {
              stopped = true;
              COMETLOG("Successfully stopped process ID via taskkill: " + pid, LoggerLevel::INFO);
            }
            else
            {
              COMETLOG("Failed to stop process ID via taskkill: " + pid, LoggerLevel::INFO);
            }
          }
#else
                // macOS/Linux: use kill (SIGTERM)
                int result = kill(processId, SIGTERM);
                if (result == 0) {
              stopped = true;
                    COMETLOG("Successfully stopped process ID: " + pid, LoggerLevel::INFO);
                } else {
                    COMETLOG("Failed to stop process ID: " + pid, LoggerLevel::CRITICAL);
                }
#endif
        }
        if (stopped)
        {
          std::string query = "UPDATE jobs SET status= " + std::to_string(convertJobStatus(JobStatus::Failed)) +
                    ", RunProgress = 'Stopped: ' || RunProgress "
                    ", ProcessId = NULL "
                    " WHERE ProcessId = '" + pid + "';";

          auto results = db.getQueryResults(query);
          COMETLOG("UPDATE THE Job Status to Stopped process ID: " + pid, LoggerLevel::INFO);
        } else {
          COMETLOG("Did not Stop process ID: " + pid, LoggerLevel::INFO);
        }
      }
      catch (const std::exception& e)
      {
        COMETLOG("Error stopping process ID: " + pid + " - " + e.what(), LoggerLevel::CRITICAL);
      }
    }
  }


  void Job::deleteJobs(const Database& db, Json::Value& jobs)
  {
    std::string whereclause = "";
    for (const auto& job : jobs)
    {
      std::string caseNumber = job["casenumber"].asString();
      std::string groupName = job["groupname"].asString();
      if (whereclause.empty())
      {
        whereclause = " WHERE ";
      }
      else
      {
        whereclause += " OR ";
      }
      whereclause += "(CaseNumber = '" + caseNumber + "' AND GroupName = '" + groupName + "')";
    }
    std::string query = "DELETE FROM jobs " + whereclause + ";";
    auto numberDeleted = db.deleteQuery("Delete jobs from database", query);
    COMETLOG("Number deleted: " + std::to_string(numberDeleted), comet::INFO);
  }

  void Job::restartJobs(const Database& db, Json::Value& jobs)
  {
    std::string whereclause = "";
    for (const auto& job : jobs)
    {
      std::string caseNumber = job["casenumber"].asString();
      std::string groupName = job["groupname"].asString();
      if (whereclause.empty())
      {
        whereclause = " WHERE ";
      }
      else
      {
        whereclause += " OR ";
      }
      whereclause += "(CaseNumber = '" + caseNumber + "' AND GroupName = '" + groupName + "')";
    }
    std::string query = "UPDATE jobs SET status=" + std::to_string(JobStatus::Queued) + ", ProcessId = null, "
      + " RunProgress='', Ranking = null, life = NULL, IterationsComplete = NULL, Servant = NULL "
      + whereclause + ";";
    auto numberUpdated = db.updateQuery("Restart jobs update", query);
    COMETLOG("Number of jobs updated to restart: " + std::to_string(numberUpdated), comet::INFO);
  }

  bool Job::startJob(Database& db, std::map<std::string, std::string>& job,
                     std::map<std::string, std::string>& servant)
  {
    COMETLOG("🏃Job:Starting job " + job["GroupName"] + " " + job["CaseNumber"]
             + " on this servant. ", comet::INFO);
    std::string executableEngine = servant["engineFolder"] + job["EngineVersion"];
    auto workingDirectory = job["WorkingDirectory"];;
    // Make sure the working directory exists
    if (!std::filesystem::exists(workingDirectory))
    {
      COMETLOG("Creating working directory: " + workingDirectory, comet::INFO);
      std::filesystem::create_directories(workingDirectory);
    }
    else
    {
      COMETLOG("Working directory already exists: " + workingDirectory, comet::INFO);
    }
    auto inputFileName = job["InputFileName"];;
    // run executable;
    COMETLOG("Job: Executing job with Engine: " + executableEngine + " in directory: " + workingDirectory +
             " with input file: " + inputFileName, comet::INFO);
    int processId = runExecutable(db, job, servant);
    COMETLOG("Process ID: " + std::to_string(processId) + " Case Number: " + job["CaseNumber"], comet::INFO);

    // Update job on manager
    nlohmann::json json;
    json["GroupName"] = job["GroupName"];;
    json["CaseNumber"] = job["CaseNumber"];;
    json["Status"] = int(JobStatus::Running);
    json["Servant"] = servant["ipAddress"];
    json["LastUpdate"] = "datetime('now')"; // This will be updated by the database
    json["RunProgress"] = "Job started on this servant.";
    json["ProcessId"] = std::to_string(processId);
    // Update the job in the manager
    if (servant["ipAddress"] == getMachineName())
    {
      auto res = Job::processUpdateRunningToManager(db, JobStatus::Running,
                                                    servant["ipAddress"],
                                                    std::to_string(processId), json["RunProgress"],
                                                    job["CaseNumber"], job["GroupName"]);
      COMETLOG("Updated this job status:, updated " + std::to_string(res) + " record", comet::DEBUGGING);
    }
    else
    {
      auto url = "http://" + servant["ipAddress"] + ":" + servant["port"] + "/job/status/database/update";
      auto result = Curl::postJson(url, json);
      COMETLOG("Updated manager job status: " + result.body, comet::DEBUGGING);
    }


    return true;
  }

  int Job::runExecutable(Database& db, std::map<std::string, std::string>& job,
                         std::map<std::string, std::string>& servant)
  {
    try
    {
      auto workDirectory = job["WorkingDirectory"];
      auto inputFileName = job["InputFileName"];

      if (!std::filesystem::exists(workDirectory))
      {
        COMETLOG("Creating working directory: " + workDirectory, comet::DEBUGGING);
        std::filesystem::create_directories(workDirectory);
      }
      else
      {
        COMETLOG("Working directory already exists: " + workDirectory, comet::DEBUGGING);
      }

      std::promise<int> pidPromise;
      auto pidFuture = pidPromise.get_future();

      std::string fullEnginePath = servant["engineFolder"] + "/" + job["EngineVersion"];
      std::string savedCaseNumber = job.count("CaseNumber") ? job["CaseNumber"] : "No case number";

      std::thread([=, &db, &pidPromise]() mutable
      {
        try
        {
          std::string managerIp = servant["managerIpAddress"].empty()
                                    ? getMachineName()
                                    : servant["managerIpAddress"];

          std::string args =
            "\"" + fullEnginePath + "\" \"" + workDirectory + inputFileName + "\""
            " --hub-progress-url \"http://" + managerIp + ":" + servant["port"] + "/job/progress\""
            " --output-dir \"" + workDirectory + "/\""
            " --phase-dir \"" + job["PhaseFileLocation"] + "/\""
            " --email \"" + job["CreatorXEmail"] + "\""
            " --code \"" + job["CreatorXCode"] + "\""
            " --case-number \"" + job["CaseNumber"] + "\""
            " --lowercase-phase-file-paths false";

#ifdef _WIN32
          STARTUPINFOA si = {sizeof(STARTUPINFOA)};
          PROCESS_INFORMATION pi;
          si.dwFlags = STARTF_USESHOWWINDOW;
          si.wShowWindow = SW_HIDE;

          char* cmdLine = _strdup(args.c_str());
          BOOL result = CreateProcessA(
            NULL, cmdLine, NULL, NULL, FALSE,
            CREATE_NO_WINDOW, NULL, NULL, &si, &pi
          );

          if (!result)
          {
            COMETLOG("Failed to start process: " + args, comet::CRITICAL);
            pidPromise.set_value(-1);
            return;
          }

          CloseHandle(pi.hThread);
          pidPromise.set_value(static_cast<int>(pi.dwProcessId));

          // Optionally wait for the process to finish
          WaitForSingleObject(pi.hProcess, INFINITE);

          DWORD exitCode = 0;
          GetExitCodeProcess(pi.hProcess, &exitCode);
          CloseHandle(pi.hProcess);

          COMETLOG("Process completed with exit code: " + std::to_string(exitCode), comet::INFO);
          Job::processUpdateRunningToManager(db, JobStatus::Completed, servant["ipAddress"],
                                             "", "Complete (" + std::to_string(exitCode) + ")", savedCaseNumber,
                                             job["GroupName"]);

#else // Unix
                pid_t pid = fork();
                if (pid == -1) {
                    COMETLOG("Failed to fork", comet::CRITICAL);
                    pidPromise.set_value(-1);
                    return;
                }

                if (pid == 0) {
                    int devNull = open("/dev/null", O_WRONLY);
                    if (devNull != -1) {
                        dup2(devNull, STDOUT_FILENO);
                        dup2(devNull, STDERR_FILENO);
                        close(devNull);
                    }

                    execl(fullEnginePath.c_str(),
                          fullEnginePath.c_str(),
                          (workDirectory + inputFileName).c_str(),
                          "--hub-progress-url",
                          (managerIp + ":" + servant["port"] + "/job/progress").c_str(),
                          "--output-dir", (workDirectory + "/").c_str(),
                          "--phase-dir", (job.at("PhaseFileLocation") + "/").c_str(),
                          "--email", job.at("CreatorXEmail").c_str(),
                          "--code", job.at("CreatorXCode").c_str(),
                          "--case-number", job.at("CaseNumber").c_str(),
                          "--lowercase-phase-file-paths", "false",
                          nullptr);

                    COMETLOG("Failed to exec: " + fullEnginePath, comet::CRITICAL);
                    _exit(EXIT_FAILURE);
                }

                pidPromise.set_value(pid);

                int status = 0;
                waitpid(pid, &status, 0);

                if (WIFEXITED(status)) {
                    int code = WEXITSTATUS(status);
                    COMETLOG("Child exited with code: " + std::to_string(code), comet::INFO);
                    Job::processUpdateRunningToManager(db, JobStatus::Completed, servant["ipAddress"],
                        "", "Complete (" + std::to_string(code) + ")", savedCaseNumber, job["GroupName"]);
                } else if (WIFSIGNALED(status)) {
                    int sig = WTERMSIG(status);
                    COMETLOG("Child terminated by signal: " + std::to_string(sig), comet::CRITICAL);
                    Job::processUpdateRunningToManager(db, JobStatus::Failed, servant["ipAddress"],
                        "", "Terminated by signal: " + std::to_string(sig), savedCaseNumber, job["GroupName"]);
                }
#endif
        }
        catch (const std::exception& e)
        {
          COMETLOG("Exception in thread: " + std::string(e.what()), comet::CRITICAL);
          pidPromise.set_value(-1);
        }
      }).detach();

      Servant::initialiseAllServantActiveCores(db);
      return pidFuture.get();
    }
    catch (const std::exception& e)
    {
      COMETLOG("Exception in runExecutable: " + std::string(e.what()), comet::CRITICAL);
      Servant::initialiseAllServantActiveCores(db);
      return -1;
    }
  }


  bool Job::processUpdateRunningToManager(
    Database& db
    , const JobStatus& aStatus
    , const std::string& aServant
    , const std::string& aProcessId
    , const std::string& aRunProgress
    , const std::string& aCaseNumber, const std::string& aGroupName
  )
  {
    // Build json object of inputs to send to the manager
    nlohmann::json json;
    json["GroupName"] = aGroupName;
    json["CaseNumber"] = aCaseNumber;
    json["Status"] = int(aStatus);
    json["Servant"] = aServant;
    json["RunProgress"] = aRunProgress;
    json["ProcessId"] = aProcessId;

    // Update the job in the manager
    if (Servant::thisServant->getManagerIpAddress().empty())
    {
      bool updateSuccess = Job::processUpdateRunning(db, aStatus, aServant, aProcessId, aRunProgress, aCaseNumber,
                                                     aGroupName);

      return true;
    }
    auto url = "http://" + Servant::thisServant->getManagerIpAddress() + ":" + std::to_string(
      Servant::thisServant->getPort()) + "/job/process_update";
    // Send the JSON to the manager
    std::thread([url, json]()
    {
      std::this_thread::sleep_for(std::chrono::seconds(1)); // Wait for 1 second
      auto result = Curl::postJson(url, json);
      if (result.status != 200 || result.body.find("success") == std::string::npos)
      {
        COMETLOG("Failed to update job status on manager: " + result.body, comet::CRITICAL);
      }
    }).detach();
    return true;
  }

  bool Job::processUpdateRunning(Database& db
                                 , const JobStatus& newStatus
                                 , const std::string& aServant
                                 , const std::string& aProcessId
                                 , const std::string& newRunProgress
                                 , const std::string& aCaseNumber
                                 , const std::string& aGroupName
  )
  {
    auto aStatus = newStatus;
    std::string processIDstring = "";
    std::string aRunProgress = newRunProgress;
    if (aStatus == JobStatus::Failed || aStatus == JobStatus::Completed || aProcessId == "")
    {
      processIDstring = "NULL";
      COMETLOG("Job completion detected: " + jobStatusDescription(aStatus) + " for #" + aCaseNumber, comet::INFO);
    }
    else
    {
      processIDstring = "'" + aProcessId + "'";
    }
    if (aStatus == JobStatus::Completed)
    {
      auto querySelect = std::string("SELECT * FROM  jobs  ") +
        "WHERE CaseNumber = '" + aCaseNumber + "' "
        "AND GroupName = '" + aGroupName + "';";
      auto result = db.getQueryResults(querySelect);
      if (result.size() > 0)
      {
        if (result[0]["Status"] != std::to_string(int(JobStatus::Running)))
        {
          aRunProgress = result[0]["RunProgress"];
          aStatus = convertJobStatus(stoi(result[0]["Status"]));
        }
      }
    }
    auto query = std::string("UPDATE jobs SET ")
      + "  Status = " + std::to_string(aStatus) + " "
      + ", Servant = '" + aServant + "' "
      + ", LastUpdate = datetime('now') "
      + ", ProcessId = " + processIDstring + " "
      + ", RunProgress = '" + aRunProgress + "' "
      "WHERE CaseNumber = '" + aCaseNumber + "' "
      "AND GroupName = '" + aGroupName + "';";
    auto rowsImpacted = db.updateQuery("Update Job Status", query, false);

    return rowsImpacted == 1;
  }

  void Job::startJobOnServant(Database& db, std::map<std::string, std::string>& job,
                              std::map<std::string, std::string>& servant)
  {
    COMETLOG("Job: Start job " + job.at("GroupName") + " " + job.at("CaseNumber")
             + " on servant: " + servant["ipAddress"] + ":" + servant["port"], comet::INFO);
    //


    if (servant["ipAddress"] == getMachineName())
    {
      auto engineFolder = servant["engineFolder"];
      startJob(db, job, servant);
    }
    else
    {
      nlohmann::json jsonData;
      // loop through add all the job map of strings and add to jsonData;
      for (const auto [label, value] : job)
      {
        jsonData[label] = value;
      }
      auto url = "http://" + servant["ipAddress"] + ":" + servant["port"] + "/job/start";
      std::list<std::string> headers;
      Curl::postJson(url, jsonData, headers);
    }
  }

  std::string Job::getJobProcessId(const std::map<std::string, std::string>& jobMap)
  {
    std::vector<std::string> keys = {
      "ProcessId", "GroupName", "CaseNumber", "CaseName", "CreatorName", "CreatorMachine"
    };
    // Join all these firlds if they exist
    std::string result = "";
    for (const auto& key : keys)
    {
      auto it = jobMap.find(key);
      if (it != jobMap.end())
      {
        if (!result.empty())
        {
          result += " - ";
        }
        result += it->second;
      }
    }
    return result;
  }

  int Job::mockRunJobs(const Database& db)
  {
    auto query = "UPDATE Jobs set Status = '" + std::to_string(int(JobStatus::Running)) + "'" +
      ", Servant = 'MockServant', ProcessId = 123456, Ranking = 123.457, Life = 45.67, IterationsComplete = 0, RunTimeMin = 12.34, RunProgress = 'Completed Iteration 0', LastUpdate = datetime('now') "
      +
      "WHERE  CaseNumber  = 20.000012 ;";
    auto rowsImpacted = db.updateQuery("Reset Running Jobs", query, false);

    auto queryQueued = "UPDATE Jobs set Status = " + std::to_string(int(JobStatus::Queued)) +
      ", Servant = NULL, Ranking = NULL, Life = NULL, IterationsComplete = NULL, RunTimeMin = NULL, RunProgress = '', LastUpdate = datetime('now'), ProcessId='' "
      +
      "WHERE  CaseNumber  = 20.000011 or CaseNumber  = 20.000025 or CaseNumber  = 20.00025 ;";
    rowsImpacted += db.updateQuery("Reset Queued Jobs", queryQueued, false);
    if (rowsImpacted == 0)
    {
      COMETLOG("Failed to reset running jobs", comet::CRITICAL);
      return 0;
    }
    return rowsImpacted;
  }

  int Job::resetRunningJobs(Database& db)
  {
    auto query = "UPDATE Jobs set Status = " + std::to_string(int(JobStatus::Queued)) +
      ", Servant = NULL, Ranking = NULL, Life = NULL, IterationsComplete = NULL, RunTimeMin = NULL, RunProgress = '', LastUpdate = datetime('now') "
      +
      "WHERE  CaseNumber = 20.000005 OR CaseNumber = 20.000011 OR CaseNumber = 20.000025  OR CaseNumber = 20.000025 ;";
    auto rowsImpacted = db.updateQuery("Reset Running Jobs", query, false);
    if (rowsImpacted == 0)
    {
      COMETLOG("No running jobs reset", comet::CRITICAL);
      return 0;
    }
    return rowsImpacted;
  }

  bool Job::updateAllInLocalDatabase(Database& db) const
  {
    auto query = getFullReplaceQueryString();

    COMETLOG("Executing query: " + query, LoggerLevel::DEBUGGING);
    auto result = db.executeQuery(query);
    if (!result)
    {
      COMETLOG("Failed to execute database query: " + query, LoggerLevel::CRITICAL);
      return false; // Or handle the error appropriately
    }

    return true;
  }


  bool Job::updateJobProgress(
    const std::string& caseNumber,
    const std::string& groupName,
    const std::string& runProgress,
    double ranking,
    double life,
    int iterationsComplete,
    const std::string& updateAt,
    int status,
    Database& db)
  {
    try
    {
      std::string processId = "";
      if (status == static_cast<int>(JobStatus::Completed) || status == static_cast<int>(JobStatus::Failed))
      {
        processId = ", ProcessId = NULL "; // Set processId to NULL if the job is completed or failed
      }
      // Update the job progress in the database
      auto query = "UPDATE jobs SET "
        "RunProgress = '" + runProgress + "', "
        "Ranking = " + std::to_string(ranking) + ", "
        "Life = " + std::to_string(life) + ", "
        "IterationsComplete = " + std::to_string(iterationsComplete) + ", "
        "LastUpdate = '" + updateAt + "', "
        "Status = " + std::to_string(status) + " " +
        processId +
        "WHERE CaseNumber = '" + caseNumber + "' AND GroupName = '" + groupName + "'";

      bool updateSuccess = db.insertRecord(query);

      return updateSuccess;
    }
    catch (const std::exception& e)
    {
      COMETLOG("Exception occurred while updating job progress: " + std::string(e.what()), LoggerLevel::CRITICAL);
      return false;
    }
  }

  std::string Job::getAllJobStatuses(Database& db, std::string& GroupName)
  {
    std::string result;
    std::string query = "Select * from jobs where GroupName = '" + GroupName + "' order by LastUpdate Limit 500;";
    auto results = db.getQueryResults(query);
    if (results.empty())
    {
      result = "No jobs found.";
      return result;
    }

    result =
      R"(Group\tCase#\tCase Name\tIteration\tNPV\tLife\tProcessId\tStatus\tProgress\tRun Time\tUpdated\tServant\tCreator\tEngine\tWorking Directory\n)";

    for (const auto& row : results)
    {
      auto aStatus = static_cast<JobStatus>(stoi(row.at("Status")));
      std::string lookupkey = "";
      // Get the following Group\tCase#\tCase Name\tIteration\tNPV\tLife\tProcessId\tStatus\tProgress\tRun Time\tUpdated\tServant\tCreator\tEngine\tWorking Directory\n
      try
      {
        std::vector<std::string> keys = {
          "GroupName", "CaseNumber", "CaseName", "IterationsComplete", "Ranking",
          "Life", "ProcessId", "Status", "RunProgress", "RunTimeMin", "LastUpdate",
          "Servant", "CreatorName", "EngineVersion", "WorkingDirectory"
        };

        std::string separator = "";
        for (const auto& key : keys)
        {
          lookupkey = key; // Update lookupkey before accessing the row
          auto value = row.at(key);
          if (key == "Status")
          {
            value = Job::jobStatusDescription(aStatus);
          }
          result += separator + value;
          separator = R"(\t)";
        }

        result += R"(\n)";

        result += "\n"; // Add a newline at the end of the row
      }
      catch (const std::exception& e)
      {
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
    switch (aStatus)
    {
    case Initialised: return "Initialised";
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
