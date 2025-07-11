//
// Created by Brett King on 7/6/2025.
//

#ifndef JOB_H
#define JOB_H
#include <string>
#include <nlohmann/json.hpp>

#include "Servant.h"
#include "drogon/HttpRequest.h"

namespace comet
  {
    class Database;

    enum JobStatus
      {
        Initialised,
        Queued,
        Allocated,
        Running,
        Failed,
        Completed,
        Unknown = -1
      };
    

    // Array with JobStatus descriptions
    static const char *JobStatusDescriptions[] = {
      "Initialised",
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
        Job(const drogon::HttpRequestPtr &request, JobStatus newStatus, Database & db);

        JobStatus jobStatus() const;
        bool validJobStatus() const;
        JobStatus setJobStatus(JobStatus newStatus);
        std::string getFullReplaceQueryString() const;
        std::string description();

        static void deleteJobs(const Database & db, Json::Value & jobs);

        static void restartJobs(const Database & db, Json::Value & jobs);

        static std::string getAllJobStatuses(Database &db, std::string &GroupName);
        static bool createNewJobsTable(Database &db);
        static bool runningProcessUpdate(Database &db, nlohmann::json &json);
        static std::string getJobProcessId(const std::map<std::string, std::string> &jobMap);
        static int mockRunJobs(const Database & db);
        static int resetRunningJobs(Database &db);
        
        static void startJobOnServant(Database &db, std::map<std::string, std::string> &job, std::map<std::string, std::string> &servant);
        static bool startJob(Database &db, std::map<std::string, std::string> &job, std::map<std::string, std::string> &servant);
        static int runExecutable(std::map<std::string, std::string> &job, std::map<std::string, std::string> &servant);
        bool updateAllInLocalDatabase(Database &db) const;

        static bool updateJobProgress(const std::string &caseNumber, const std::string &groupName,
                               const std::string &runProgress,
                               double ranking, double life, int iterationsComplete, const std::string &updateAt,
                               int status,
                               Database &db);

        static std::string jobStatusDescription(JobStatus aStatus);
        static std::string htmlJobSummaryReport(Database &db, std::string &sort, std::string &filter);
      };
  };
#endif //JOB_H
