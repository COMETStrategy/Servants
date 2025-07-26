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
        DEBUGGING,
        INFO,
        WARNING,
        CRITICAL,
        NONE
      };

    std::string LogLevelToString(LoggerLevel level);
    std::string formatTime(std::time_t time);
    #define COMETLOG(message, LoggerLevel) comet::Logger::log(message, LoggerLevel, __FILE__, __LINE__)

    class   Logger
      {
      private:
        static std::string m_fileName;
        static LoggerLevel m_LoggerLevel;

      public:
        // Use >> operator overload to write to the COMETLOG file
        static void log(const std::string &message, LoggerLevel COMETLogLevel, const char *file, int line);

        static void setFileName(const std::string &fileName);
        static const std::string getFileName();

        static void setLoggerLevel(const LoggerLevel &level);
        static LoggerLevel getLoggerLevel(const LoggerLevel &level);
      };


  } // cs

#endif //LOGGER_H
