//
// Created by Brett King on 5/6/2025.
//

#include "Logger.h"

#include <chrono>
#include <iomanip>
#include <iostream>

namespace comet
  {
    // comet

    std::string comet::Logger::m_fileName = "";
    LoggerLevel comet::Logger::m_logLevel = LoggerLevel::INFO;

    std::string LogLevelToString(LoggerLevel level)
      {
        switch (level) {
          case LoggerLevel::ALL: return "ALL";
          case LoggerLevel::INFO: return "INFO";
          case LoggerLevel::DEBUG: return "DEBUG";
          case LoggerLevel::WARNING: return "WARNING";
          case LoggerLevel::CRITICAL: return "CRITICAL";
          case LoggerLevel::NONE: return "NONE";
          default: return "UNKNOWN";
        }
      }

    std::string formatTime(std::time_t time) {
        std::tm* utcTime = std::gmtime(&time); // Use UTC
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", utcTime);
        return std::string(buffer);
    }

    void comet::Logger::log(const std::string &message, const LoggerLevel logLevel)
      {
        {
          if (logLevel >= Logger::m_logLevel) {
            std::string timeStamp = formatTime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
            if (logLevel < LoggerLevel::WARNING)
              std::cout << "COMET LOGGER " << timeStamp << " [" << LogLevelToString(logLevel) << "] " << message << std::endl << std::flush;
            else              
              std::cerr << "COMET LOGGER " << timeStamp << " [" << LogLevelToString(logLevel) << "] " << message << std::endl << std::flush;
          }
        }
      }

    void comet::Logger::setFileName(const std::string &fileName)
      {
        m_fileName = fileName;
      }

    const std::string Logger::getFileName()
      {
        return m_fileName;
      }

    void comet::Logger::setLoggerLevel(const LoggerLevel &level)
      {
        m_logLevel = level;
      }

    LoggerLevel Logger::getLoggerLevel(const LoggerLevel &level)
      {
        return m_logLevel;
      }
  }
