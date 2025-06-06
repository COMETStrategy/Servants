//
// Created by Brett King on 5/6/2025.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <string>

namespace comet
  {
    enum LoggerLevel
      {
        ALL,
        INFO,
        DEBUG,
        WARNING,
        CRITICAL,
        NONE
      };

    class Logger
      {
      private:
        static std::string m_fileName;
        static LoggerLevel m_logLevel;

      public:
        // Use >> operator overload to write to the log file
        static void log(const std::string &message, LoggerLevel logLevel = LoggerLevel::INFO);

        static void setFileName(const std::string &fileName);

        static void setLoggerLevel(const LoggerLevel &level);
      };
  } // cs

#endif //LOGGER_H
