//
// Created by Brett King on 7/6/2025.
//

#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include <stdexcept>
#include <sqlite3.h>

namespace comet
  {
    class Database
      {
      public:
        // Constructor: Opens the database
        explicit Database(const std::string &dbPath);

        // Destructor: Closes the database
        ~Database();

        // Run an SQL query
        void executeQuery(const std::string &query);

      private:
        sqlite3 *m_db = nullptr; // SQLite database handle
        std::string m_dbPath;
      };
  } // comet

#endif //DATABASE_H
