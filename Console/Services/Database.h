//
// Created by Brett King on 7/6/2025.
//

#ifndef DATABASE_H
#define DATABASE_H
#include <map>
#include <string>
#include <stdexcept>
#include <sqlite3.h>

namespace comet
  {
    class Database
      {
      public:
        void createTableIfNotExists(std::string tableName, std::string insertQuery);

        void insertRecord(std::string insertQuery);

        bool createNewDatabase(const std::string & path);

        bool updateQuery(const std::string &description, const std::string &queryText) const;

        bool openDatabase(const std::string & databaseFullPath);

        // Constructor: Opens the database
        explicit Database(const std::string &dbPath);

        // Destructor: Closes the database
        ~Database();

        // Run an SQL query
        bool executeQuery(const std::string &query) const;

        std::vector<std::map<std::string, std::string>> getQueryResults(const std::string &query);

        bool isConnected();

      private:
        sqlite3 *m_db = nullptr; // SQLite database handle
        std::string m_dbPath;
      };
  } // comet

#endif //DATABASE_H
