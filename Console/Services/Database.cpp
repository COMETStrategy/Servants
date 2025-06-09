//
// Created by Brett King on 7/6/2025.
//
#include <filesystem>
#include <vector>
#include <string>

#include "Logger.h"
#include "Database.h"

#include <map>

namespace comet {

    Database::Database(const std::string &dbPath) {
        std::string expandedPath = dbPath;
        if (dbPath[0] == '~') {
          const char *home = std::getenv("HOME");
          if (home) {
            expandedPath = std::string(home) + dbPath.substr(1);
          }
        }

        auto parentPath = std::filesystem::path(expandedPath).parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
          comet::Logger::log("Database directory does not exist. Creating directory: " + parentPath.string(), LoggerLevel::INFO);
          std::filesystem::create_directories(parentPath);
        }

        if (!std::filesystem::exists(expandedPath)) {
          comet::Logger::log("Database file " + expandedPath + " does not exist. Creating a new database.", LoggerLevel::INFO);
          if (sqlite3_open(expandedPath.c_str(), &m_db) == SQLITE_OK) {
            // Create a table called version with the single field created and set this to the current time
            std::string createTableQuery = "CREATE TABLE IF NOT EXISTS version (createdDate TEXT, lastUpdatedDate TEXT);";
            char *errMsg = nullptr;
            if (sqlite3_exec(m_db, createTableQuery.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
              comet::Logger::log("Failed to create version table: " + std::string(errMsg), LoggerLevel::CRITICAL);
              sqlite3_free(errMsg);
              throw std::runtime_error("Failed to create version table");
            } else {
              comet::Logger::log("Version table created successfully.", LoggerLevel::INFO);
              // Insert current date and time into the version table
              std::string insertQuery = "INSERT INTO version (createdDate, lastUpdatedDate) VALUES (DATETIME('now'), DATETIME('now'));";
              if (sqlite3_exec(m_db, insertQuery.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
                comet::Logger::log("Failed to insert record into version table: " + std::string(errMsg), LoggerLevel::CRITICAL);
                sqlite3_free(errMsg);
                throw std::runtime_error("Failed to insert record into version table");
              } else {
                comet::Logger::log("Record inserted into version table successfully.", LoggerLevel::INFO);
              }
            }
          }
        }

        if (sqlite3_open(expandedPath.c_str(), &m_db) != SQLITE_OK) {
          comet::Logger::log("Failed to open database: " + std::string(sqlite3_errmsg(m_db)), LoggerLevel::CRITICAL);
          throw std::runtime_error("Failed to open database");
        } else {
          
          char *errMsg = nullptr;
          sqlite3_stmt *stmt = nullptr;
          std::string selectQuery = "SELECT createdDate, lastUpdatedDate FROM version LIMIT 1;";
          auto results = getQueryResults("SELECT createdDate, lastUpdatedDate FROM version;");
          
          if (sqlite3_prepare_v2(m_db, selectQuery.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
              std::string createdDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
              std::string lastUpdatedDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
              comet::Logger::log("Existing Database creation: " + createdDate + ", last updated at: " + lastUpdatedDate, LoggerLevel::DEBUG);
            } else {
              comet::Logger::log("No records found in version table.", LoggerLevel::WARNING);
            }
            sqlite3_finalize(stmt);
          } else {
            comet::Logger::log("Failed to retrieve current values: " + std::string(sqlite3_errmsg(m_db)), LoggerLevel::CRITICAL);
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to retrieve current values");
          }

          std::string updateQuery = "UPDATE version SET lastUpdatedDate = DATETIME('now');";
          if (sqlite3_exec(m_db, updateQuery.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            comet::Logger::log("Failed to update record into version table: " + std::string(errMsg), LoggerLevel::CRITICAL);
            sqlite3_free(errMsg);
            throw std::runtime_error("Failed to update record into version table");
          } else {
            comet::Logger::log("Record updated into version table successfully.", LoggerLevel::DEBUG);
          }
          
          comet::Logger::log("Database file '" + expandedPath + "' opened.", LoggerLevel::INFO);
          m_dbPath = expandedPath;
        }
    }


    
    Database::~Database() {
        if (m_db) {
          sqlite3_close(m_db);
        }
    }

    void Database::executeQuery(const std::string &query) {
        char *errMsg = nullptr;
        if (sqlite3_exec(m_db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
          std::string error = "SQL error: " + std::string(errMsg);
          comet::Logger::log(error, LoggerLevel::CRITICAL);
          sqlite3_free(errMsg);
          throw std::runtime_error(error);
        }
    }



    std::vector<std::map<std::string, std::string>> Database::getQueryResults(const std::string &query) {
        sqlite3_stmt *stmt = nullptr;
        std::vector<std::map<std::string, std::string>> results;
        // Debug log the query being executed
        comet::Logger::log("Executing query: " + query, LoggerLevel::DEBUG);

        if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
          while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string, std::string> row;
            int columnCount = sqlite3_column_count(stmt);
            for (int i = 0; i < columnCount; ++i) {
              const char *columnName = sqlite3_column_name(stmt, i);
              const char *columnText = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
              row[columnName ? columnName : ""] = columnText ? columnText : "";
            }
            results.push_back(row);
          }
          sqlite3_finalize(stmt);
        } else {
          comet::Logger::log("Failed to execute query: " + std::string(sqlite3_errmsg(m_db)), LoggerLevel::CRITICAL);
          sqlite3_finalize(stmt);
          throw std::runtime_error("Failed to execute query");
        }

        comet::Logger::log("Number of rows retrieved: " + std::to_string(results.size()), LoggerLevel::DEBUG);
        return results;
    }
    
} // comet