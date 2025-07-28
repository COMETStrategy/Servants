//
// Created by Brett King on 29/7/2025.
//

#ifndef ROUTES_H
#define ROUTES_H


#include <drogon/drogon.h>
#include "Servant.h"
#include "Database.h"

namespace comet
  {
    class Routes
      {
      private:
        static Servant aServant;
        static Database db;
        static bool m_running;
        
        static void handleInvalidMethod(const drogon::HttpRequestPtr &request);

      public:
        Routes(Servant & servant, Database &database, bool &running);  
        static void Alive(const drogon::HttpRequestPtr &request,
        std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void Quit(const drogon::HttpRequestPtr &request,
                                 std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        static std::string htmlSetBody(const std::string &body, const std::string &targetPath, const std::string &title);
      };
  }

#endif //ROUTES_H
