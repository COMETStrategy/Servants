//
// Created by Brett King on 5/6/2025.
//

#ifndef LOGGER_H
#define LOGGER_H
#include <ctime>
#include <string>

namespace comet
  {
    enum LoggerLevel
      {
        ALL,
        DEBUG,
        INFO,
        WARNING,
        CRITICAL,
        NONE
      };

    std::string LogLevelToString(LoggerLevel level);
    std::string formatTime(std::time_t time);
    #define LOG(message, logLevel) comet::Logger::log(message, logLevel, __FILE__, __LINE__)

    class Logger
      {
      private:
        static std::string m_fileName;
        static LoggerLevel m_logLevel;

      public:
        // Use >> operator overload to write to the log file
        static void log(const std::string &message, LoggerLevel logLevel = LoggerLevel::INFO, const char *file = __FILE__, int line =
                            __LINE__);

        static void setFileName(const std::string &fileName);
        static const std::string getFileName();

        static void setLoggerLevel(const LoggerLevel &level);
        static LoggerLevel getLoggerLevel(const LoggerLevel &level);
      };
  } // cs

#endif //LOGGER_H
