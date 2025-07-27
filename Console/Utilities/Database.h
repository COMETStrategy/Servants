//
// Created by Brett King on 7/6/2025.
//

#ifndef DATABASE_H
#define DATABASE_H
#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <sqlite3.h>

namespace comet
  {
    class Database
      {
      public:
        void createTableIfNotExists(std::string tableName, std::string insertQuery);
        bool insertRecord(std::string insertQuery, bool logErrors = true);

        int deleteQuery(const std::string &description, const std::string &queryText, bool logErrors = true) const;

        int updateQuery(const std::string &description, const std::string &queryText, bool logErrors = true) const;
        bool openDatabase(const std::string & databaseFullPath);

        // Constructor: Opens the database
        explicit Database(const std::string &dbPath);
        // Destructor: Closes the database
        ~Database();

        // Run an SQL query
        bool executeQuery(const std::string &query) const;
        std::vector<std::map<std::string, std::string>> getQueryResults(const std::string &query) const;

        bool isConnected();
        bool databaseExists;
        const bool databaseAlreadyExists() const { return databaseExists; }
        sqlite3 *getDatabaseHandle() const { return m_db; }
        void closeDatabase()
          {
            if (m_db) {
              sqlite3_close(m_db);
              m_db = nullptr;
            }
          }
        
        

      private:
        sqlite3 *m_db = nullptr; // SQLite database handle
        std::string m_dbPath;
      };
  } // comet

#endif //DATABASE_H
