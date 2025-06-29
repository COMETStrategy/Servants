//
// Created by Brett King on 7/6/2025.
//
#include <filesystem>
#include <vector>
#include <string>

#include "Logger.h"
#include "Database.h"

#include <map>

#include "utilities.h"
#include "../../../../../../opt/homebrew/Cellar/openssl@3/3.5.0/include/openssl/obj_mac.h"


#define CREATE_TABLE_SETTINGS_QUERY R"(CREATE TABLE IF NOT EXISTS settings
                                      (createdDate TEXT
                                      , lastUpdatedDate TEXT
                                      , email TEXT
                                      , code TEXT
                                      , machineId TEXT
                                      , totalCores INTEGER
                                      , unusedCores INTEGER
                                      , managerIpAddress TEXT);
                                      )"



namespace comet
  {
    Database::Database(const std::string &dbPath)
      {
        auto fullPath = getFullFilenameAndDirectory(dbPath);
        
        databaseExists = openDatabase(fullPath);

      }


    Database::~Database()
      {
        if (m_db) {
          sqlite3_close(m_db);
        }
      }

    void Database::createTableIfNotExists(std::string tableName, std::string insertQuery)
      {
        char *errMsg;
        // Insert jobs table
        if (sqlite3_exec(m_db, insertQuery.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
          comet::Logger::log("Failed to create table " + tableName + ": " + std::string(errMsg),
                             LoggerLevel::CRITICAL);
          sqlite3_free(errMsg);
          throw std::runtime_error("Failed to create " + tableName + " table");
        } else {
          comet::Logger::log("Table " + tableName + " created successfully.", LoggerLevel::INFO);
        }
      }

    bool Database::insertRecord(std::string insertQuery, bool logErrors)
      {
        char *errMsg;
        if (sqlite3_exec(m_db, insertQuery.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
          if (logErrors)
            comet::Logger::log("Failed to insert record into Settings table: " + std::string(errMsg),
                               LoggerLevel::CRITICAL);
          sqlite3_free(errMsg);
          return false; // Return false to indicate failure
        } else {
          if (logErrors) comet::Logger::log("Record inserted into Settings table successfully.", LoggerLevel::DEBUG);
          return true;
        }
      }


    int Database::updateQuery(const std::string &description, const std::string &queryText, bool logErrors) const
      {
        char *errMsg;
        if (sqlite3_exec(m_db, queryText.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
          if (logErrors) comet::Logger::log("Failed to update: " + description + std::string(errMsg),
                             LoggerLevel::CRITICAL);
          sqlite3_free(errMsg);
          throw std::runtime_error("Failed to update record into Settings table: " + std::string(errMsg));
        } else {
          if (logErrors) comet::Logger::log("Record updated into Settings table successfully.", LoggerLevel::DEBUG);
        }
        return sqlite3_changes(m_db);

      }

    bool Database::openDatabase(const std::string &databaseFullPath)
      {
        bool databaseExists = std::filesystem::exists(databaseFullPath);

        if (sqlite3_open(databaseFullPath.c_str(), &m_db) != SQLITE_OK) {
          comet::Logger::log("Failed to open database: " + std::string(sqlite3_errmsg(m_db)), LoggerLevel::CRITICAL);
          return false;
        }
        comet::Logger::log("Database file '" + databaseFullPath + "' opened successfully.", LoggerLevel::DEBUG);
        
        m_dbPath = databaseFullPath;
        return databaseExists;
      }

    bool Database::executeQuery(const std::string &query) const
      {
        char *errMsg = nullptr;
        if (sqlite3_exec(m_db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
          std::string error = "SQL error: " + std::string(errMsg);
          comet::Logger::log(error, LoggerLevel::CRITICAL);
          sqlite3_free(errMsg);
          throw std::runtime_error(error);
          return false;
        }
        return true;
      }


    std::vector<std::map<std::string, std::string> > Database::getQueryResults(const std::string &query)
      {
        sqlite3_stmt *stmt = nullptr;
        std::vector<std::map<std::string, std::string> > results;
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

    bool Database::isConnected()
      {
        if (m_db) {
          // Set pCur and pCurUsed to nullptr
          int current = 0, highwater = 0;
          int result = sqlite3_db_status(m_db, SQLITE_DBSTATUS_LOOKASIDE_USED, &current, &highwater, 0);
          comet::Logger::log("sqlite3_db_status result: " + std::to_string(result), LoggerLevel::DEBUG);
          if (result == SQLITE_OK || result == SQLITE_BUSY || result == SQLITE_DBSTATUS_LOOKASIDE_USED) {
            comet::Logger::log("Database is connected.", LoggerLevel::DEBUG);
            return true;
          }
        }
        comet::Logger::log("Database is not connected.", LoggerLevel::WARNING);
        return false;
      }
  } // comet
