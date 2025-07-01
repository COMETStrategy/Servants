//
// Created by Brett King on 1/7/2025.
//
#include <map>

#include "Scheduler.h"

#include "Servant.h"
#include "Database.h"
#include "Logger.h"

namespace comet
  {
    Scheduler::Scheduler()
      {
      }

    void Scheduler::initialise()
      {
        // Update the number of jobs running
      }

    Scheduler::~Scheduler()
      {
        jobs.clear();
      }

    int Scheduler::startJobsOnBestServants(Database &db)
      {
        int numberOfJobsStarted = 0;

        auto servents = Servant::getAvailableServants(db);
        if (servents.size() == 0) return 0;

        auto jobs = db.getQueryResults("SELECT * FROM jobs WHERE status = 0 ORDER BY lastUpdate ASC;");
        if (jobs.empty()) {
          COMETLOG("No jobs available to start.", LoggerLevel::INFO);
          return 0;
        }
        for (const auto &job: jobs) {
          // Find the best servant for the job
          for (const auto &servant: servents) {
            auto availableCores = std::stoi(servant.at("totalCores")) -
                                  std::stoi(servant.at("unusedCores")) -
                                  std::stoi(servant.at("activeCores"));
            if (availableCores > 0) {
              // Start the job on this servant
              COMETLOG(std::string("Starting job '") + job.at("GroupName") + ":"  + job.at("CaseNumber") + "' on servant '" + servant.at("ipAddress") + "'",
                       LoggerLevel::INFO);              // Update the job status to Running
              db.updateQuery("Update Job Status",
                             "UPDATE jobs SET status = 1, servant = '" + servant.at("ipAddress") +
                             "' WHERE caseNumber = '" + job.at("CaseNumber") + "' and GroupName = '" + job.at(
                               "GroupName") + "';");
              numberOfJobsStarted++;
              break; // Break after starting a job on one servant
            }
          }
        }
      }
  } // comet
