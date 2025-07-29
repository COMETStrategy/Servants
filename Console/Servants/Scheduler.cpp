//
// Created by Brett King on 1/7/2025.
//
#include <map>

#include "../Utilities/Database.h"
#include "../Utilities/Logger.h"

#include "Scheduler.h"
#include "Servant.h"
#include "Job.h"

namespace comet
  {
    Database *Scheduler::db = nullptr;
    bool Scheduler::autoStartJobs = true; // Automatically start jobs on best servants

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

    int Scheduler::startJobsOnBestServants(Database &aDatabase)
      {
        db = &aDatabase; // Set the static db pointer to the provided database
        int numberOfJobsStarted = 0;

        auto servents = Servant::getAvailableServants(*db);
        if (servents.size() == 0) return 0;

        auto jobs = db->getQueryResults(
          "SELECT * FROM jobs WHERE status <= " + std::to_string(int(JobStatus::Queued)) + " ORDER BY lastUpdate ASC;");
        if (jobs.empty()) {
          COMETLOG("No jobs available to start.", LoggerLevel::INFO);
          return 0;
        }

        auto servant = servents.begin();
        int availableCores = 0;
        if (servant != servents.end()) {
          //COMETLOG("Servant IP Address: " + servant->at("ipAddress"), LoggerLevel::INFO);
          availableCores = std::stoi(servant->at("totalCores")) -
                           std::stoi(servant->at("unusedCores")) -
                           std::stoi(servant->at("activeCores"));
        }

        for (auto &job: jobs) {
          // Find the best servant for the job

          while (availableCores == 0 && servant != servents.end() - 1) {
            ++servant; // Move to the next servant
            availableCores = std::stoi(servant->at("totalCores")) -
                             std::stoi(servant->at("unusedCores")) -
                             std::stoi(servant->at("activeCores"));
            //COMETLOG(
            //  "Servant " + servant->at("ipAddress") + " has " + std::to_string(availableCores) + " available cores.",
            //  LoggerLevel::INFO);
          }
          if (availableCores > 0) {
            // Start the job on this servant
            COMETLOG(
              std::string("Scheduler: Starting job '") + job.at("GroupName") + ":" + job.at("CaseNumber") +
              "' on servant '" +
              servant->at("ipAddress")  + "'",
              LoggerLevel::DEBUGGING); // Update the job status to Running
            db->updateQuery("Update Job Status",
                            "UPDATE jobs "
                            " SET status = " + std::to_string(JobStatus::Allocated)
                            + ", servant = '" + servant->at("ipAddress") + "'" 
                            + " WHERE caseNumber = '" + job.at("CaseNumber") + "'"
                            + " AND  GroupName = '" + job.at("GroupName") + "'"
                            ";");
            db->updateQuery("Update Servant State",
                            "UPDATE servants SET activeCores = activeCores+1 WHERE ipAddress = '" + servant->at(
                              "ipAddress") + "' ;");
            Job::startJobOnServant(*db, job, *servant);

            numberOfJobsStarted++;
            availableCores--;
          } else {
            COMETLOG(std::string("Cannot process any more jobs ") + std::to_string(numberOfJobsStarted)
                     + " of " + std::to_string(jobs.size()) + " started.",
                     LoggerLevel::INFO);
            break;
          }
        }

        COMETLOG(std::string("✅ Started ") + std::to_string(numberOfJobsStarted)
                 + " of " + std::to_string(jobs.size()) + " jobs.", LoggerLevel::INFO);
        return numberOfJobsStarted;
      }
  } // comet
