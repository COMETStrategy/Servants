//
// Created by Brett King on 29/7/2025.
//

#ifndef ROUTES_H
#define ROUTES_H


#include <drogon/drogon.h>
#include "Authenticator.h"
#include "Servant.h"
#include "Database.h"

namespace comet
  {
    class Routes
      {
      private:
        static Authenticator auth;
        static Servant aServant;
        static Database db;
        static bool m_running;
        
        static void handleInvalidMethod(const drogon::HttpRequestPtr &request);

      public:
        Routes(Authenticator &anAuth, Servant & servant, Database &database, bool &running);

        void registerAllHandlers();

        static void Alive(const drogon::HttpRequestPtr &request,std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void Authenticate(const drogon::HttpRequestPtr &request,std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void Authentication(const drogon::HttpRequestPtr &request,
                                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);;
        static void Quit(const drogon::HttpRequestPtr &request,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        static std::string htmlSetBody(const std::string &body, const std::string &targetPath, const std::string &title);
        static std::string htmlHeader(const std::string &fullTargetPath, const std::string &title) ;
        static std::string htmlFooter() ;
      };
  }

#endif //ROUTES_H
