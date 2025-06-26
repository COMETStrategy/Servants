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

#define CREATE_TABLE_SERVANTS_QUERY R"(CREATE TABLE IF NOT EXISTS `servants` (
                                      `nameIpAddress` varchar(250) NOT NULL
                                      , `projectId` int NOT NULL
                                      , `registrationTime` datetime NOT NULL
                                      , `lastUpdateTime` datetime NOT NULL
                                      , `ServantVersion` varchar(30) NOT NULL
                                      , `email` varchar(100) NOT NULL
                                      , `code` varchar(100)NOT NULL
                                      , `ListeningPort` int NOT NULL
                                      , `totalCores` int NOT NULL
                                      , `unusedCores` int NOT NULL
                                      , PRIMARY KEY(`nameIpAddress`, `projectId`)
                                      );)"


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

#define CREATE_TABLE_JOBS_QUERY R"(
CREATE TABLE IF NOT EXISTS jobs (
LastUpdate DATETIME NOT NULL,
GroupName VARCHAR(1000) NOT NULL,
CaseNumber TEXT NOT NULL,
Status TINYINT NOT NULL,
CaseName VARCHAR(1000) NOT NULL,
Id DOUBLE NOT NULL,
Servant VARCHAR(100),
IterationsComplete INT,
Ranking DOUBLE,
Life DOUBLE,
RunTimeMin DOUBLE,
RunProgress VARCHAR(2000) NOT NULL,
CreatorName VARCHAR(100) NOT NULL,
CreatorMachine VARCHAR(100) NOT NULL,
CreatorXEmail VARCHAR(100) NOT NULL,
CreatorXCode VARCHAR(100) NOT NULL,
peopleId INT NOT NULL,
projectId INT NOT NULL,
InputFileName VARCHAR(100) NOT NULL,
EngineVersion TEXT NOT NULL,
EngineDirectory VARCHAR(1000) NOT NULL,
PhaseFileLocation TEXT,
WorkingDirectory VARCHAR(1000) NOT NULL,
CaseBody TEXT NOT NULL,
Id_temp INTEGER PRIMARY KEY AUTOINCREMENT,
UNIQUE (GroupName, CaseNumber, projectId)
);
)"

#define CREATE_INDEXES_JOBS_QUERY R"(
CREATE INDEX IF NOT EXISTS idx_Id ON jobs (Id);
CREATE INDEX IF NOT EXISTS idx_GroupNameOnly ON jobs (GroupName);
CREATE INDEX IF NOT EXISTS idx_Node ON jobs (Servant);
CREATE INDEX IF NOT EXISTS idx_projectId ON jobs (projectId);
CREATE INDEX IF NOT EXISTS idx_peopleId ON jobs (peopleId);
CREATE INDEX IF NOT EXISTS idx_UserCase ON jobs (CaseNumber);
)"

namespace comet
  {
    Database::Database(const std::string &dbPath)
      {
        auto fullPath = getFullFilenameAndDirectory(dbPath);
        auto databaseExists = openDatabase(fullPath);


        if (!databaseExists) {
          databaseExists = createNewDatabase(fullPath);
        }

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
          if (logErrors) comet::Logger::log("Failed to insert record into Settings table: " + std::string(errMsg),
                             LoggerLevel::CRITICAL);
          sqlite3_free(errMsg);
          return false; // Return false to indicate failure
        } else {
          if (logErrors) comet::Logger::log("Record inserted into Settings table successfully.", LoggerLevel::DEBUG);
          return true;
        }
      }

    bool Database::createNewDatabase(const std::string &path)
      {
        comet::Logger::log("Database file " + path + " does not exist. Creating a new database.",
                           LoggerLevel::INFO);

        createTableIfNotExists("Settings",CREATE_TABLE_SETTINGS_QUERY);
        insertRecord("INSERT INTO settings (CreatedDate, lastUpdatedDate, machineid, totalCores, unusedCores, manageripAddress) VALUES (DATETIME('now'), DATETIME('now'), 'machinenumber', 0, 0, '');");

        createTableIfNotExists("Settings",CREATE_TABLE_SERVANTS_QUERY);
        
        createTableIfNotExists("Jobs",CREATE_TABLE_JOBS_QUERY);
        createTableIfNotExists("Jobs Indexes",CREATE_INDEXES_JOBS_QUERY);

        comet::Logger::log("Settings and Jobs tables created successfully.", LoggerLevel::INFO);
        return true;
      }

    bool Database::updateQuery(const std::string &description, const std::string &queryText) const
      {
        char *errMsg;
        if (sqlite3_exec(m_db, queryText.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
          comet::Logger::log("Failed to update record into Settings table: " + std::string(errMsg),
                             LoggerLevel::CRITICAL);
          sqlite3_free(errMsg);
          throw std::runtime_error("Failed to update record into Settings table: " + std::string(errMsg));
        } else {
          comet::Logger::log("Record updated into Settings table successfully.", LoggerLevel::DEBUG);
        }
        
        return true;
      }

    bool Database::openDatabase(const std::string &databaseFullPath)
      {
        bool databaseExists = std::filesystem::exists(databaseFullPath);

        if (sqlite3_open(databaseFullPath.c_str(), &m_db) != SQLITE_OK) {
          comet::Logger::log("Failed to open database: " + std::string(sqlite3_errmsg(m_db)), LoggerLevel::CRITICAL);
          return false;
        }
        comet::Logger::log("Database file '" + databaseFullPath + "' opened successfully.", LoggerLevel::DEBUG);


        // Existing database
        if (!databaseExists) {
          return createNewDatabase(databaseFullPath);
        }

        // Database exists so update the last access time
        sqlite3_stmt *stmt = nullptr;
        // Update the lastUpdatedDate in the Settings table        
        auto bSuccess = updateQuery("Update Settings", "UPDATE Settings SET lastUpdatedDate = DATETIME('now');");

        comet::Logger::log("Database file '" + databaseFullPath + "' opened.", LoggerLevel::DEBUG);
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
