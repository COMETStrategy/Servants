//
// Created by Brett King on 7/6/2025.
//

#ifndef JOB_H
#define JOB_H
#include <string>
#include <json/json.h>

#include "drogon/HttpRequest.h"

namespace comet
  {
    class Database;

    enum JobStatus
      {
        Queued,
        Allocated,
        Running,
        Failed,
        Completed,
        Unknown = -1
      };

    // Array with JobStatus descriptions
    static const char *JobStatusDescriptions[] = {
      "Queued",
      "Allocated",
      "Running",
      "Failed",
      "Completed"
    };

    class Job
      {
      private:
        // Members
        std::string email;
        std::string code;
        std::string inputFileName;
        std::string timeStamp;
        std::string timeout;
        std::string type;
        std::string caseNumber;
        std::string title;
        std::string caseName;
        std::string creatorMachine;
        std::string engineVersion;
        std::string engineDirectory;
        std::string basePhaseDirectory;
        std::string workingDirectory;
        std::string caseBody;
        std::string creatorName; // Replace with actual creator name
        std::string creatorXEmail;
        std::string creatorXCode;
        int peopleRefValue;
        int projectRefValue;


        JobStatus status;

        // Constructor
      public:
        Job(const drogon::HttpRequestPtr &request);

        JobStatus jobStatus() const;

        bool validJobStatus() const;

        std::string jobStatusDescription() const;

        JobStatus setJobStatus(const char *statusDescription);

        std::string getReplaceQueryString() const;

        static bool createNewJobsTable(Database &db);

        std::string description();
      };
  };
#endif //JOB_H
