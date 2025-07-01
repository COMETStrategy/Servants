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

      public:
        Scheduler();
        void initialise();

        ~Scheduler();

        int startJobsOnBestServants(Database &db);
      };


  } // comet

#endif //SCHEDULER_H
