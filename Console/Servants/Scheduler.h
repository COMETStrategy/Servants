//
// Created by Brett King on 1/7/2025.
//

#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <vector>

namespace comet
  {
    class Job;
    class Servant;
    class Database;

    class Scheduler
      {
      private:
        std::vector<Job *> jobs;
        static bool autoStartJobs; // Automatically start jobs on best servants

      public:
        Scheduler();
        void initialise();
        static bool getAutoStartJobs() { return autoStartJobs; }
        static void setAutoStartJobs(bool autoStart) { autoStartJobs = autoStart; }

        ~Scheduler();

        static int startJobsOnBestServants(Database &db);
        static Database *db;
      };


  } // comet

#endif //SCHEDULER_H
