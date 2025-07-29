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
    LoggerLevel comet::Logger::m_LoggerLevel = LoggerLevel::INFO;

    std::string LogLevelToString(LoggerLevel level)
      {
        switch (level) {
          case LoggerLevel::ALL: return "ALL";
          case LoggerLevel::INFO: return "INFO";
          case LoggerLevel::DEBUGGING: return "DEBUGGING";
          case LoggerLevel::WARNING: return "WARNING";
          case LoggerLevel::CRITICAL: return "CRITICAL";
          case LoggerLevel::NONE: return "NONE";
          default: return "UNKNOWN";
        }
      }

    std::string formatTime(std::time_t time)
      {
        std::tm *utcTime = std::gmtime(&time); // Use UTC
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", utcTime);
        return std::string(buffer);
      }

    #if defined(_WIN32) && defined(_DEBUG)
      #include <windows.h>
      #include <string>

    struct DebugStreamBuf : public std::streambuf {
      std::string buffer;
      int overflow(int c) override {
        if (c == '\n' || c == EOF) {
          if (!buffer.empty()) {
            OutputDebugStringA(buffer.c_str());
            buffer.clear();
          }
        }
        else {
          buffer += static_cast<char>(c);
        }
        return c;
      }
      int sync() override {
        if (!buffer.empty()) {
          OutputDebugStringA(buffer.c_str());
          buffer.clear();
        }
        return 0;
      }
    };

    #endif

    void Logger::log(const std::string &message, const LoggerLevel COMETLOGLevel, const char *file, int line)
      {
        // Extract the file name using string manipulation
        std::string filePath = file;
        size_t lastSlash = filePath.find_last_of("/\\");
        size_t secondLastSlash = filePath.find_last_of("/\\", lastSlash - 1);

        std::string lastDirectoryAndFile = (secondLastSlash != std::string::npos)
                                             ? filePath.substr(secondLastSlash + 1)
                                             : filePath;
        std::string fileLocation = "" + lastDirectoryAndFile + "(" + std::to_string(line) + "): ";

        std::string timeStamp = formatTime(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        std::string commonText = "COMET Logger " + timeStamp + " [" + LogLevelToString(COMETLOGLevel) + "] " + message +
          " " + fileLocation;

        if (COMETLOGLevel >= Logger::m_LoggerLevel) {
          if (COMETLOGLevel < LoggerLevel::WARNING)
            std::cout << commonText << std::endl << std::flush;
          else
            std::cerr << commonText << std::endl << std::flush;
        }
#if defined(_WIN32) && defined(_DEBUG)


        std::string debugText = fileLocation + " COMET Logger " + timeStamp + " [" + LogLevelToString(COMETLOGLevel) + "] " + message + "";

        DebugStreamBuf debugStreamBuf;
        std::ostream debugStream(&debugStreamBuf); 

        OutputDebugStringA(debugText.c_str()); 
        OutputDebugStringA("\n");
#endif
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
        m_LoggerLevel = level;
      }

    LoggerLevel Logger::getLoggerLevel(const LoggerLevel &level)
      {
        return m_LoggerLevel;
      }
  }
