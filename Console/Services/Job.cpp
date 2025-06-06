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

Job::Job(const std::string &emailHeader, const std::string &codeHeader, const Json::Value &json) {
    try {
        if (emailHeader.empty()) {
            throw std::invalid_argument("Email header is missing");
        }
        if (codeHeader.empty()) {
            throw std::invalid_argument("Code header is missing");
        }
        if (!json.isMember("InputFileName") || !json["InputFileName"].isString()) {
            throw std::invalid_argument("Missing or invalid 'InputFileName' in JSON");
        }
        if (!json.isMember("TimeStamp") || !json["TimeStamp"].isString()) {
            throw std::invalid_argument("Missing or invalid 'TimeStamp' in JSON");
        }
        // Additional checks...

        email = emailHeader;
        code = codeHeader;
        inputFileName = json["InputFileName"].asString();
        timeStamp = json["TimeStamp"].asString();
        timeout = json["timeout"].asString();
        type = json["type"].asString();
        caseNumber = json["attributes"]["CaseNumber"].asString();
        title = json["attributes"]["Title"].asString();
        caseName = json["attributes"]["CaseName"].asString();
        creatorMachine = json["attributes"]["CreatorMachine"].asString();

        // Decode Base64 and apply UTF-8 encoding
        engineVersion = comet::Encoding::utf8_encode(json["attributes"]["EngineVersion"].asString());
        engineDirectory64 = comet::Encoding::utf8_encode(comet::Encoding::decode(json["attributes"]["EngineDirectory64"].asString()));
        basePhaseDirectory64 = comet::Encoding::utf8_encode(comet::Encoding::decode(json["attributes"]["BasePhaseDirectory64"].asString()));
        workingDirectory64 = comet::Encoding::utf8_encode(comet::Encoding::decode(json["attributes"]["WorkingDirectory64"].asString()));
        caseBody64 = json["attributes"]["CaseBody64"].asString();
    } catch (const std::invalid_argument &e) {
        comet::Logger::log(e.what()); // Log the error message
        status = Failed; // Set job status to Failed
        throw; // Re-throw the exception if needed
    }

    status = Queued; // Set initial job status to Queued

    comet::Logger::log("✅ " + jobStatusDescription() + " : " + title + " #" + caseNumber + " (" + caseName + ") "); // Log the job status
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
      case Processing: return "Processing";
      case Completed: return "Completed";
      case Failed: return "Failed";
      default: return "Unknown Status";
    }
  }



