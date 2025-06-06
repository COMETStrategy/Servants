//
// Created by Brett King on 7/6/2025.
//

#ifndef JOB_H
#define JOB_H
#include <string>
#include <json/json.h>

enum JobStatus
  {
    Queued,
    Processing,
    Completed,
    Failed
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
    std::string engineDirectory64;
    std::string basePhaseDirectory64;
    std::string workingDirectory64;
    std::string caseBody64;

    JobStatus status;

    // Constructor
    public:
      Job(const std::string &emailHeader, const std::string &codeHeader, const Json::Value &json);
      JobStatus jobStatus() const;
      bool validJobStatus() const;
      std::string jobStatusDescription() const;
      
  };
#endif //JOB_H
