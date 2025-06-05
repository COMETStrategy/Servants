//
// Created by Brett King on 5/6/2025.
//

#include "Logger.h"

#include <iostream>

namespace comet
  {
    // comet

    std::string comet::Logger::m_fileName = "";
    LoggerLevel comet::Logger::m_logLevel = LoggerLevel::INFO;

    void comet::Logger::log(const std::string &message, const LoggerLevel logLevel)
      {
        {
          if (logLevel >= Logger::m_logLevel) {
            std::cout << "[" << logLevel << "] " << message << std::endl;
          }
        }
      }

    void comet::Logger::setFileName(const std::string &fileName)
      {
        m_fileName = fileName;
      }

    void comet::Logger::setLoggerLevel(const LoggerLevel &level)
      {
        m_logLevel = level;
      }
  }
