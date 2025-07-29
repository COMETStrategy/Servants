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

        static void Alive(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void Authenticate(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void Authentication(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void Configuration(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void ExecuteCommand(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobProcessUpdate(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobSelectedStop(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobSelectedDelete(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobSelectedRestart(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobStart(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobStatusDatabaseUpdate(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobSummary(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void JobProgress(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback);
        static void MockRunJobs(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void Quit(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void ResetRunningJobs(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void RunQueued(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void ServantSelectedDelete(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void ServantSettings(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void ServantStatus(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void ServantStopProcesses(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void ServantSummary(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void UpdateAliveServants(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);
        static void UploadJob(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        static std::string htmlSetBody(const std::string &body, const std::string &targetPath, const std::string &title);
        static std::string htmlHeader(const std::string &fullTargetPath, const std::string &title) ;
        static std::string htmlFooter() ;
      };
  }

#endif //ROUTES_H
