//
// Created by Brett King on 17/6/2025.
//

#ifndef UTILITIES_H
#define UTILITIES_H
#include <cstdlib>
#include <filesystem>
#include <string>

#include "Logger.h"

namespace comet
  {

    // Function to get the full filename and directory, creating directories if necessary

    inline std::string getFullFilenameAndDirectory(const std::string &filename)
      {
        if (filename.empty()) {
          return "";
        }

        // Check if the filename is already an absolute path
        if (filename[0] == '/' || (filename.size() > 2 && filename[1] == ':' && filename[2] == '\\')) {
          return filename; // Already an absolute path
        }

        std::string fullPath = filename;
        if (filename[0] == '~') {
          const char *home = std::getenv("HOME");
          if (home) {
            fullPath = std::string(home) + filename.substr(1);
          }
        }

        // Get the parent directory
        std::filesystem::path parentPath = std::filesystem::path(fullPath).parent_path();
        // Log it
        comet::Logger::log("Parent path: " + parentPath.string(), comet::LoggerLevel::INFO);
        if (!std::filesystem::exists(parentPath)) {
          if (!std::filesystem::create_directories(parentPath)) {
            comet::Logger::log("Failed to create database directory: " + parentPath.string(), comet::LoggerLevel::CRITICAL);
            throw std::runtime_error("Unable to create database directory: " + parentPath.string());
          }
          comet::Logger::log("Database directory created: " + parentPath.string(), comet::LoggerLevel::INFO);
        }
    
        return std::filesystem::path(fullPath);

      }

    inline std::string generateTimestamp()
      {
        return std::to_string(std::time(nullptr));
      }
  }

#endif //UTILITIES_H
