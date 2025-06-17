//
// Created by Brett King on 7/6/2025.
//

#include "Job.h"
#include "Logger.h"
#include "Encoding.h"
#include "Database.h"

#include <string>
#include <stdexcept>

#include <stdexcept>

#include "utilities.h"


Job::Job(const drogon::HttpRequestPtr &request)
  {
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
      comet::Logger::log(e.what(),comet::INFO); // Log the error message
      status = JobStatus::Failed; // Set job status to Failed
    }

    status = JobStatus::Queued; // Set initial job status to Queued
  // Log the job status
  }

std::string Job::getReplaceQueryString() const
  {

    auto jobId = comet::generateTimestamp(); // Member function to generate timestamp
    return std::string("") + "REPLACE INTO jobs (GroupName, CaseNumber, projectId, LastUpdate, "
                               "CaseName, ID, CreatorName, CreatorXEmail, CreatorXCode, CreatorMachine, peopleId, "
                               "InputFileName, EngineVersion, EngineDirectory, PhaseFileLocation, WorkingDirectory, "
                               "Status, RunProgress, Servant, Ranking, Life, IterationsComplete, RunTimeMin, CaseBody) "
                               "VALUES ('" + title + "', " + caseNumber + ", " + std::to_string(
                                 projectRefValue) + ", datetime('now'), "
                               "'" + caseName + "', '" + jobId + "', '" + creatorName + "', '" + creatorXEmail +
                               "', '" + creatorXCode + "', '" + creatorMachine + "', " + std::to_string(peopleRefValue) +
                               ", "
                               "'" + inputFileName + "', '" + engineVersion + "', '" + engineDirectory + "', '" +
                               basePhaseDirectory + "', '" + workingDirectory + "', "
                               + std::to_string(status) +
                               ", '', NULL, NULL, NULL, NULL, NULL, '" + caseBody + "');";
  }

std::string Job::description()
  {
    return jobStatusDescription() + " : " + title + " #" + caseNumber + " (" + caseName + ") ";
  }

JobStatus Job::jobStatus() const
  {
    return status;
  }

bool Job::validJobStatus() const
  {
    return status != JobStatus::Failed;
  }

std::string Job::jobStatusDescription() const
  {
    // Convert JobStatus to string description
    switch (status) {
      case Queued: return "Queued";
      case Allocated: return "Allocated";
      case Running: return "Running";
      case Failed: return "Failed";
      case Completed: return "Completed";
      default: return "Unknown Status";
    }
  }

JobStatus Job::setJobStatus(const char *statusDescription)
  {
    // update the status from the description using a switch statement
    if (strcmp(statusDescription, "Queued") == 0) {
      status = Queued;
    } else if (strcmp(statusDescription, "Allocated") == 0) {
      status = Allocated;
    } else if (strcmp(statusDescription, "Running") == 0) {
      status = Running;
    } else if (strcmp(statusDescription, "Failed") == 0) {
      status = Failed;
    } else if (strcmp(statusDescription, "Completed") == 0) {
      status = Completed;
    } else {
      status = Unknown;
      //throw std::invalid_argument("Invalid job status description");
    }
    return status;

  }


