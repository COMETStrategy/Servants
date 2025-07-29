//
// Created by Brett King on 29/7/2025.
//


#include <nlohmann/json.hpp>
#include <string>
#include <stdexcept>

#include "Routes.h"

#include "../Utilities//Logger.h"
#include "Job.h"
#include "Scheduler.h"

using namespace drogon;

namespace comet
  {
    // Initialise static members
    Authenticator Routes::auth;
    Servant Routes::aServant;
    Database Routes::db;
    bool Routes::m_running = true;
    
    void Routes::registerAllHandlers()
      {
        std::vector<PossibleRoutes> allPossibleRoutes = {
             {true, false, "/", Routes::Alive, {drogon::Get, drogon::Post}},
             {true, false, "/alive", Routes::Alive, {drogon::Get, drogon::Post}},
             {true, false, "/authenticate", Routes::Authenticate, {drogon::Get, drogon::Post}},
             {true, false, "/authentication", Routes::Authentication, {drogon::Get, drogon::Post}},
             {true, false, "/configuration", Routes::Configuration, {drogon::Post}},
             {true, false, "/execute_command", Routes::ExecuteCommand, {drogon::Post}},
             {true, false, "/job/process_update", Routes::JobProcessUpdate, {drogon::Post}},
             {true, false, "/job/progress", Routes::JobProgress, {drogon::Get, Post}},
             {true, false, "/jobs/selected_delete", Routes::JobSelectedDelete, {drogon::Post}},
             {true, false, "/jobs/selected_restart", Routes::JobSelectedRestart, {drogon::Post}},
             {true, false, "/jobs/selected_stop", Routes::JobSelectedStop, {drogon::Post}},
             {true, false, "/job/start", Routes::JobStart, {drogon::Post}},
             {true, false, "/job/status_database_update", Routes::JobStatusDatabaseUpdate, {drogon::Post}},
             {true, false, "/job/summary", Routes::JobSummary, {drogon::Get}},
             {true, true, "/mockrunjobs", Routes::MockRunJobs, {drogon::Post, drogon::Get}},
             {true, true, "/quit", Routes::Quit, {drogon::Get, drogon::Post}},
             {true, false, "/resetrunningjobs", Routes::ResetRunningJobs, {drogon::Post, drogon::Get}},
             {true, false, "/run_queued", Routes::RunQueued, {drogon::Post, drogon::Get}},
             {true, false, "/servant/selected_delete", Routes::ServantSelectedDelete, {drogon::Post}},
             {true, false, "/servant/stop_processes", Routes::ServantStopProcesses, {drogon::Post}},
             {true, false, "/servant/status", Routes::ServantStatus, {drogon::Post}},
             {true, false, "/servant/summary", Routes::ServantSummary, {drogon::Get}},
             {true, false, "/servant/settings", Routes::ServantSettings, {drogon::Get, drogon::Post}},
             {true, false, "/updateAliveServants", Routes::UpdateAliveServants, {drogon::Post, drogon::Get}},
             {true, false, "/upload/job", Routes::UploadJob, {drogon::Post}},
        };

        for (const auto &r: allPossibleRoutes) {
          auto chosen = r.handler;
          bool isAuthenticated = auth.machineAuthenticationisValid();
          bool isManager = aServant.isManager();
          // isDebug
          #if defined(_GLIBCXX_DEBUG) || defined(DEBUG)
            bool inDebug = true;
          #else
            bool inDebug = false;
          #endif
          
          if ((r.showInDebug && inDebug) || (!r.showInDebug))
          {
            drogon::app().registerHandler(r.path, [r](const drogon::HttpRequestPtr &req,
                                                      std::function<void(const drogon::HttpResponsePtr &)> &&callback)
                                            {
                                              r.handler(req, std::move(callback));
                                            }, r.methods);

            COMETLOG(std::string("Registering route: ") + r.path , LoggerLevel::DEBUGGING);
          }
          else {
            COMETLOG(std::string("IGNORED route: ") + r.path , LoggerLevel::DEBUGGING);
          }
        }


        //app().registerHandler("/alive", &Routes::Alive, {Get, Post});
      }
    
    void Routes::handleInvalidMethod(const HttpRequestPtr &request)
      {
        std::string upperMethod = to_string(request->method());
        std::transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), ::toupper);
        // Get the request body
        //std::string body = std::string(request->getBody());

        COMETLOG("Handling " + upperMethod + " request to " + request->path(), LoggerLevel::DEBUGGING);
      }

    Routes::Routes(Authenticator &anAuth, Servant &servant, Database &database, bool &running)
      {
        auth = anAuth;
        aServant = servant;
        db = database;
        m_running = running;

        registerAllHandlers();
      }

    void Routes::Alive(
      const drogon::HttpRequestPtr &request,
      std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);
        if (request->method() == drogon::Post) {
          auto host = request->getHeader("Host");
          std::string hubAddress = "http://" + aServant.getIpAddress() + ":" + std::to_string(aServant.getPort()) + "/";
          nlohmann::json jsonResponse = {
            {"company", "COMET Strategy - Australia"},
            {"HubName", "COMET Servant (" + aServant.getIpAddress() + "):LOCAL"},
            {"HubAddress", hubAddress},
            {"CloudDataConnection", "notused"},
            {"ErrorMessage", ""}
          };

          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setContentTypeCode(CT_APPLICATION_JSON);
          resp->setBody(jsonResponse.dump());
          callback(resp);
          return;
        }

        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(
          htmlSetBody(
            "Alive"
            , ""
            , "Test COMET Servant Alive")
        );
        COMETLOG("Proof of life: " + request->getHeader("Host") + " is Alive", LoggerLevel::DEBUGGING);
        callback(resp);
      }

    void Routes::Authenticate(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        aServant.setEmail(request->getParameter("email"));
        aServant.setCode(request->getParameter("code"));
        //aServant.setIpAddress(request->getParameter("ipAddress"));

        const bool isAuthenticated = auth.valid(aServant.getEmail(), aServant.getCode(), aServant.getIpAddress());
        if (!isAuthenticated) {
          COMETLOG("Invalid authentication parameters", LoggerLevel::CRITICAL);

          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k302Found);
          resp->addHeader("Location", "/authentication");
          COMETLOG(
            std::string("Authentication ") + ((isAuthenticated) ? "successful ✅" : "failed ❌") + " for email: " +
            aServant
            .getEmail() +
            ". Redirecting to /",
            LoggerLevel::INFO);
          callback(resp);
          return;
        }

        aServant.updateServantSettings(db);

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k302Found);
        resp->addHeader("Location", "/servant/settings");
        COMETLOG(
          std::string("✅ Authentication ") + ((isAuthenticated) ? "successful" : "failed") + " for email: " +
          aServant
          .getEmail() +
          ". Redirecting to /",
          LoggerLevel::INFO);
        callback(resp);
      }


    void Routes::Authentication(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);
        if (request->method() == drogon::Post) {
          auto host = request->getHeader("Host");
          std::string hubAddress = "http://" + host + "/";

          nlohmann::json jsonResponse = {
            {"company", "COMET Strategy - Australia"},
            {"HubName", "COMET Servant (" + aServant.getIpAddress() + "):LOCAL"},
            {"HubAddress", hubAddress},
            {"CloudDataConnection", "notused"},
            {"ErrorMessage", ""}
          };

          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setContentTypeCode(CT_APPLICATION_JSON);
          resp->setBody(jsonResponse.dump());
          callback(resp);
          return;
        }

        // GET response
        auto resp = HttpResponse::newHttpResponse();
        std::string responseBody = aServant.htmlAuthenticationSettingsForm(auth);

        resp->setBody(htmlSetBody(responseBody, "/authentication", "Servant Authentication"));
        COMETLOG("Servant Home: alive!", LoggerLevel::INFO);
        callback(resp);
      }

    void Routes::RunQueued(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        if (aServant.isManager()) {
          COMETLOG(
            "This Servant does not run queued jobs since it is not the managing Servant, submit to " + aServant.getManagerIpAddress(),
            LoggerLevel::DEBUGGING);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k403Forbidden);
          resp->setBody(R"({"ErrorMessage":"This servant is not authorized to run queued jobs"})");
          callback(resp);
          return;
        }

        try {
          Scheduler::setAutoStartJobs(true);
          int jobsStarted = Scheduler::startJobsOnBestServants(db);

          if (jobsStarted > 0) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(k302Found);
            resp->addHeader("Location", "/?filter=active");
            COMETLOG("🏃 " + std::to_string(jobsStarted) + " queued jobs started. Redirecting to /?filter=active", LoggerLevel::INFO);
            callback(resp);
          } else {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(k200OK);
            resp->setBody(htmlSetBody("No jobs available to start or cores available to run.", "/run_queued/", "Run Queued Jobs"));
            COMETLOG("No queued jobs available to start", LoggerLevel::INFO);
            callback(resp);
          }
        } catch (const std::exception &e) {
          COMETLOG(std::string("Error starting queued jobs: ") + e.what(), LoggerLevel::CRITICAL);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(R"({"ErrorMessage":"Internal server error"})");
          callback(resp);
        }
      }

    void Routes::ServantSelectedDelete(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"ErrorMessage":"Invalid or missing JSON payload"})");
          callback(resp);
          return;
        }

        try {
          auto rows = (*json)["rows"];
          Servant::deleteServants(db, rows);

          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setBody(R"({"Status":"Servants deleted successfully"})");
          callback(resp);
        } catch (const std::exception &e) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
        }
      }

    void Routes::ServantSettings(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);


        if (!auth.machineAuthenticationisValid()) {
          COMETLOG("Unauthorized access to /", LoggerLevel::WARNING);
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k302Found);
          resp->addHeader("Location", "/authentication");
          callback(resp);
          return;
        }

        if (request->method() == drogon::Post) {
          // Handle POST request
          auto json = request->getJsonObject();
          if (!json) {
            COMETLOG("Invalid JSON payload", LoggerLevel::WARNING);
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            resp->setBody(R"({"ErrorMessage":"Invalid JSON payload"})");
            callback(resp);
            return;
          }

          try {
            //aServant.setIpAddress(request->getHeader("Host"));
            aServant.setTotalCores((*json)["totalCores"].asInt());
            aServant.setUnusedCores((*json)["unusedCores"].asInt());
            aServant.setActiveCores((*json)["activeCores"].asInt());
            aServant.setManagerIpAddress((*json)["managerIpAddress"].asString());
            aServant.setEngineFolder((*json)["engineFolder"].asString());
            aServant.setCentralDataFolder((*json)["centralDataFolder"].asString());
            aServant.setPriority((*json)["priority"].asDouble());
            aServant.updateServantSettings(db);

            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k200OK);
            resp->setBody(R"({"status":"Servant settings updated successfully"})");
            COMETLOG("Servant settings updated successfully", LoggerLevel::INFO);
            callback(resp);
          } catch (const std::exception &e) {
            COMETLOG(std::string("Error updating servant settings: ") + e.what(), LoggerLevel::CRITICAL);
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k500InternalServerError);
            resp->setBody(R"({"ErrorMessage":"Failed to update servant settings"})");
            callback(resp);
          }
          return;
        }

        // Handle GET request
        //aServant.setIpAddress(request->getHeader("Host"));
        auto responseBody = aServant.htmlServantSettingsForm();
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(htmlSetBody(responseBody, "/servant/settings",
                                  "Servant Settings: " + aServant.getIpAddress()));
        COMETLOG("Servant settings page served", LoggerLevel::INFO);
        callback(resp);
      }

    void Routes::ServantStatus(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody("Invalid JSON properties");
          callback(resp);
          return;
        }

        try {
          aServant.setTotalCores((*json)["totalCores"].asInt());
          aServant.setUnusedCores((*json)["unusedCores"].asInt());
          aServant.setActiveCores((*json)["activeCores"].asInt());
          aServant.setManagerIpAddress((*json)["managerIpAddress"].asString());
          aServant.setEngineFolder((*json)["engineFolder"].asString());
          aServant.setCentralDataFolder((*json)["centralDataFolder"].asString());
          aServant.setVersion((*json)["ServantVersion"].asString());
          aServant.setEmail((*json)["email"].asString());
          aServant.setCode((*json)["code"].asString());
          aServant.setPort((*json)["port"].asInt());
          aServant.setProjectId((*json)["projectId"].asInt());
          aServant.updateServantSettings(db);

          auto postData = nlohmann::json{{"status", "success"}, {"message", "Status updated successfully"}};
          auto resp = drogon::HttpResponse::newHttpJsonResponse(postData.dump());
          callback(resp);
        } catch (const std::exception &e) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(drogon::k400BadRequest);
          resp->setBody("Error processing JSON: " + std::string(e.what()));
          callback(resp);
        }
      }

    void Routes::ServantStopProcesses(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json || !json->isMember("processIds")) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"ErrorMessage":"Invalid or missing processIds in JSON payload"})");
          callback(resp);
          return;
        }

        try {
          auto processIds = (*json)["processIds"].asString();
          Job::stopProcessesLocally(db, processIds);

          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setBody(R"({"Status":"Processes stopped successfully"})");
          callback(resp);
        } catch (const std::exception &e) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
        }
      }

    void Routes::ServantSummary(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);
        if (!auth.machineAuthenticationisValid()) {
          COMETLOG("Unauthorized access to /servant/summary", LoggerLevel::WARNING);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k401Unauthorized);
          resp->setBody("Unauthorized");
          callback(resp);
          return;
        }

        std::string report = Servant::htmlServantSummary(db);

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setBody(htmlSetBody(report, "/servant/summary", "Servant Summary"));
        callback(resp);
      }

    void Routes::Configuration(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto totalCores = request->getParameter("totalCores");
        auto unusedCores = request->getParameter("unusedCores");
        if (unusedCores.empty()) unusedCores = "0";
        auto activeCores = request->getParameter("activeCores");
        if (activeCores.empty()) activeCores = "0";
        auto priority = request->getParameter("priority");
        if (priority.empty()) priority = "0";
        auto managerIpAddress = request->getParameter("managerIpAddress");
        auto engineFolder = request->getParameter("engineFolder");
        auto centralDataFolder = request->getParameter("centralDataFolder");
        auto alive = request->getParameter("alive");
        if (alive.empty()) alive = "0";

        aServant.setTotalCores(std::stoi(totalCores));
        aServant.setUnusedCores(std::stoi(unusedCores));
        aServant.setActiveCores(std::stoi(activeCores));
        aServant.setPriority(std::stod(priority));
        aServant.setManagerIpAddress(managerIpAddress);
        aServant.setEngineFolder(engineFolder);
        aServant.setCentralDataFolder(centralDataFolder);
        aServant.setAlive(std::stoi(alive));

        if (!db.insertRecord(
          "UPDATE servants SET totalCores = '" + totalCores + "' "
          ", unusedCores = '" + unusedCores + +"' "
          ", activeCores = '" + activeCores + +"' "
          ", managerIpAddress = '" + managerIpAddress + "' "
          ", engineFolder = '" + engineFolder + "' "
          ", centralDataFolder = '" + centralDataFolder + "' "
          ", lastUpdateTime = DATETIME('now') "
          "WHERE ipAddress = '" + aServant.getIpAddress() + "';")) {
          COMETLOG("Failed to update Settings table with configuration information: ",
                   LoggerLevel::CRITICAL);

          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->addHeader("Location", "/servant/summary");
          resp->setBody(R"({"error":"Failed to update Settings table with configuration information"})");
          callback(resp);
          return;
        }

        aServant.updateDatabase(db);
        aServant.updateManagerDatabase();

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(k302Found);
        resp->addHeader("Location", "/servant/settings");
        COMETLOG("Configuration successfully saved. Redirecting to /", LoggerLevel::INFO);
        callback(resp);
      }

    void Routes::ExecuteCommand(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json || !json->isMember("command")) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"ErrorMessage":"Invalid or missing command in JSON payload"})");
          callback(resp);
          return;
        }

        std::string command = (*json)["command"].asString();
        try {
          int result = system(command.c_str());
          if (result == 0) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k200OK);
            resp->setBody(R"({"Status":"Command executed successfully"})");
            callback(resp);
          } else {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k500InternalServerError);
            resp->setBody(R"({"ErrorMessage":"Failed to execute command"})");
            callback(resp);
          }
        } catch (const std::exception &e) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
        }
      }

    void Routes::JobProcessUpdate(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json ||
            !json->isMember("GroupName") ||
            !json->isMember("CaseNumber") ||
            !json->isMember("Status") ||
            !json->isMember("Servant") ||
            !json->isMember("RunProgress") ||
            !json->isMember("ProcessId")) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"ErrorMessage":"Invalid or missing fields in JSON payload"})");
          callback(resp);
          return;
        }

        try {
          JobStatus status = static_cast<JobStatus>(json->get("Status", 0).asInt());
          std::string servant = json->get("Servant", "").asString();
          std::string processId = json->get("ProcessId", "").asString();
          std::string runProgress = json->get("RunProgress", "").asString();
          std::string caseNumber = json->get("CaseNumber", "").asString();
          std::string groupName = json->get("GroupName", "").asString();

          bool updateSuccess = Job::processUpdateRunning(db, status, servant, processId, runProgress, caseNumber,
                                                         groupName);

          if (updateSuccess) {
            Json::Value responseJson;
            responseJson["ErrorMessage"] = "";
            responseJson["Status"] = "Job process updated successfully";
            responseJson["CaseNumber"] = caseNumber;

            auto resp = HttpResponse::newHttpJsonResponse(responseJson);
            callback(resp);
            COMETLOG("Job process updated successfully for CaseNumber: " + caseNumber, LoggerLevel::INFO);
          } else {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k500InternalServerError);
            resp->setBody(R"({"ErrorMessage":"Failed to update job process in database"})");
            callback(resp);
            COMETLOG("Failed to update job process for CaseNumber: " + caseNumber, LoggerLevel::CRITICAL);
          }
        } catch (const std::exception &e) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
          COMETLOG(std::string("Exception occurred while updating job process: ") + e.what(),
                   LoggerLevel::CRITICAL);
        }
      }


    void Routes::JobSelectedStop(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json || !json->isMember("rows")) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"ErrorMessage":"Invalid or missing JSON payload"})");
          callback(resp);
          return;
        }

        try {
          Scheduler::setAutoStartJobs(false);
          auto jobs = (*json)["rows"];
          Servant::stopSelectedProcesses(db, jobs);
          Servant::initialiseAllServantActiveCores(db);

          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setBody(R"({"Status":"Jobs stopped successfully"})");
          callback(resp);
        } catch (const std::exception &e) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
        }
      }

    void Routes::JobSelectedDelete(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"ErrorMessage":"Invalid or missing JSON payload"})");
          callback(resp);
          return;
        }

        try {
          Scheduler::setAutoStartJobs(false);
          auto jobs = (*json)["rows"];
          Job::deleteJobs(db, jobs);
          Servant::initialiseAllServantActiveCores(db);

          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setBody(R"({"Status":"Jobs deleted successfully"})");
          callback(resp);
        } catch (const std::exception &e) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
        }
      }

    void Routes::JobSelectedRestart(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        auto json = request->getJsonObject();
        if (!json) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"ErrorMessage":"Invalid or missing JSON payload"})");
          callback(resp);
          return;
        }
        auto jobs = (*json)["rows"];

        try {
          Scheduler::setAutoStartJobs(false);
          Servant::stopSelectedProcesses(db, jobs);
          Job::restartJobs(db, jobs);
          Servant::initialiseAllServantActiveCores(db);
          // Wait 1 second to allow all old cases to complete
          std::this_thread::sleep_for(std::chrono::seconds(1));
          Scheduler::setAutoStartJobs(true);
          int jobsStarted = Scheduler::startJobsOnBestServants(db);

          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setBody(R"({"Status":"Jobs reset successfully"})");
          callback(resp);
        } catch (const std::exception &e) {
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
        }
        
      }

    void Routes::JobStart(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        try {
          int jobsStarted = Scheduler::startJobsOnBestServants(db);

          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k200OK);
          resp->setBody(R"({"Status":"Jobs started successfully", "JobsStarted":)" + std::to_string(jobsStarted) + R"(})");
          callback(resp);
        } catch (const std::exception &e) {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
          callback(resp);
        }
      }

    void Routes::JobStatusDatabaseUpdate(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
                handleInvalidMethod(request);

              auto json = request->getJsonObject();
              if (!json || !json->isMember("JobId") || !json->isMember("Status")) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"ErrorMessage":"Invalid or missing JobId/Status in JSON payload"})");
                callback(resp);
                return;
              }

              try {
                auto Id = (*json)["Id"].asString();
                std::string status = (*json)["Status"].asString();

                bool updateSuccess = true; //Job::updateJobStatusInDatabase(db, jobId, status);
                if (updateSuccess) {
                  Json::Value responseJson;
                  responseJson["ErrorMessage"] = "";
                  responseJson["Status"] = "Job status updated successfully";
                  responseJson["Id"] = Id;

                  auto resp = HttpResponse::newHttpJsonResponse(responseJson);
                  callback(resp);
                  COMETLOG("Job status updated successfully for JobId: " + Id, LoggerLevel::INFO);
                } else {
                  auto resp = HttpResponse::newHttpResponse();
                  resp->setStatusCode(k500InternalServerError);
                  resp->setBody(R"({"ErrorMessage":"Failed to update job status in database"})");
                  callback(resp);
                  COMETLOG("Failed to update job status for JobId: " + Id, LoggerLevel::CRITICAL);
                }
              } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
                callback(resp);
                COMETLOG(std::string("Exception occurred while updating job status: ") + e.what(),
                         LoggerLevel::CRITICAL);
              }
      }

    void Routes::JobSummary(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);
        if (!auth.machineAuthenticationisValid()) {
          COMETLOG("Unauthorized access to /", LoggerLevel::WARNING);
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k302Found);
          resp->addHeader("Location", "/authentication");
          callback(resp);
          return;
        }

        auto sort = request->getParameter("sort");
        auto filter = request->getParameter("filter");

        std::string report = Job::htmlJobSummaryReport(db, sort, filter);

        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(htmlSetBody(report, request->path() + "?" + request->query(), "Job Summary"));
        callback(resp);

      }

    void Routes::JobProgress(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
  handleInvalidMethod(request);
              std::string body = std::string(request->body());
              //COMETLOG("Job progress: " + body, LoggerLevel::INFO);

              auto json = nlohmann::json::parse(body, nullptr, false);
              if (!json.is_object()
                  || !json.contains("CaseNumber")
                  || !json.contains("GroupName")
                  || !json.contains("RunProgress")
                  || !json.contains("Ranking")
                  || !json.contains("Life")
                  || !json.contains("IterationsComplete")
                  || !json.contains("UpdateAt")
                  || !json.contains("Status")
              ) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"ErrorMessage":"Invalid or missing fields in JSON payload"})");
                callback(resp);
                return;
              }

              try {
                double caseN = json["CaseNumber"].get<double>();
                // Convert to a 6 decimal string
                std::string caseNumber = std::to_string(caseN);
                caseNumber = caseNumber.substr(0, caseNumber.find('.') + 7); // Keep 6 decimal places
                if (caseNumber.length() < 6) {
                  caseNumber.insert(0, 6 - caseNumber.length(), '0'); // Pad with zeros if less than 6 characters
                }

                std::string groupName = json["GroupName"].get<std::string>();
                std::string runProgress = json["RunProgress"].get<std::string>();
                double ranking = json["Ranking"].get<double>();
                double life = json["Life"].get<double>();
                int iterationsComplete = json["IterationsComplete"].get<int>();
                std::string updateAt = json["UpdateAt"].get<std::string>();
                int status = json["Status"].get<int>();

                // Update the job progress in the database
                bool updateSuccess = Job::updateJobProgress(
                  caseNumber,
                  groupName,
                  runProgress,
                  ranking,
                  life,
                  iterationsComplete,
                  updateAt,
                  status,
                  db);
                if (status == JobStatus::Failed || status == JobStatus::Completed) {
                  Servant::initialiseAllServantActiveCores(db);
                  if (Scheduler::getAutoStartJobs()) Scheduler::startJobsOnBestServants(db);
                }

                if (updateSuccess) {
                  nlohmann::json responseJson = {
                    {"ErrorMessage", ""},
                    {"Status", "Job progress updated successfully"},
                    {"CaseNumber", caseNumber}
                  };

                  // Json::Value jsonResponse;
                  // for (auto &[key, value]: responseJson.items()) {
                  //   jsonResponse[key] = value.dump();
                  // }

                  auto resp = HttpResponse::newHttpResponse();
                  resp->setStatusCode(k200OK);
                  resp->setContentTypeCode(CT_APPLICATION_JSON);
                  resp->setBody(responseJson.dump());
                  callback(resp);

                  COMETLOG(
                    "Job progress updated successfully for CaseNumber: " + caseNumber + ", " + Job::jobStatusDescription
                    (convertJobStatus(status)) + ": "+ runProgress, LoggerLevel::INFO);
                } else {
                  auto resp = HttpResponse::newHttpResponse();
                  resp->setStatusCode(k500InternalServerError);
                  resp->setBody(R"({"ErrorMessage":"Failed to update job progress in database"})");
                  callback(resp);
                  COMETLOG("Failed to update job progress for CaseNumber: " + caseNumber, LoggerLevel::CRITICAL);
                }
              } catch (const std::exception &e) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(std::string(R"({"ErrorMessage":"Exception occurred: )") + e.what() + R"("})");
                callback(resp);
                COMETLOG(std::string("Exception occurred while updating job progress: ") + e.what(),
                         LoggerLevel::CRITICAL);
              }
      }

    void Routes::MockRunJobs(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        try {
          int updatedRows = Job::mockRunJobs(db);

          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k302Found);
          resp->addHeader("Location", "/");
          callback(resp);

          COMETLOG("Successfully executed mock run jobs", LoggerLevel::INFO);
        } catch (const std::exception &e) {
          COMETLOG(std::string("Error executing mock run jobs: ") + e.what(), LoggerLevel::CRITICAL);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(R"({"ErrorMessage":"Failed to execute mock run jobs"})");
          callback(resp);
        }
      }

    void Routes::ResetRunningJobs(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        try {
          int updatedRows = Job::resetRunningJobs(db);

          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k302Found);
          resp->addHeader("Location", "/");
          callback(resp);

          COMETLOG("Successfully reset running jobs to queued", LoggerLevel::INFO);
        } catch (const std::exception &e) {
          COMETLOG(std::string("Error resetting running jobs: ") + e.what(), LoggerLevel::CRITICAL);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k500InternalServerError);
          resp->setBody(R"({"ErrorMessage":"Failed to reset running jobs"})");
          callback(resp);
        }
      }

    void Routes::UpdateAliveServants(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        if (!auth.machineAuthenticationisValid()) {
          COMETLOG("Unauthorized access to /updateAliveServants", LoggerLevel::WARNING);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k401Unauthorized);
          resp->setBody("Unauthorized");
          callback(resp);
          return;
        }

        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(k302Found);
        resp->addHeader("Location", "/authentication");
        resp->setBody("Checking status of Servants and then will redirect to Servant summary...");
        callback(resp);

        Servant::checkAllServantsAlive(db);
      }

    void Routes::UploadJob(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);

        if (!aServant.isManager()) {
          COMETLOG("This Servant does not accept jobs since it is not the managing Servant, submit to " + aServant.getManagerIpAddress(), LoggerLevel::DEBUGGING);
        }

        if (!db.isConnected()) {
          COMETLOG("Database connection failed", LoggerLevel::CRITICAL);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k406NotAcceptable);
          resp->setBody(R"({"ErrorMessage":"Error mySQL database connection"})");
          callback(resp);
          return;
        }

        auto json = request->getJsonObject();
        if (!json) {
          COMETLOG("Invalid JSON payload", LoggerLevel::WARNING);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"error":"Invalid JSON payload"})");
          callback(resp);
          return;
        }

        Job aJob(request, JobStatus::Queued, db);
        if (aJob.validJobStatus()) {
          Scheduler::setAutoStartJobs(true);
          Scheduler::startJobsOnBestServants(db);
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k201Created);
          resp->setBody(R"({"status":"Job uploaded successfully"})");
          callback(resp);
        } else {
          auto resp = drogon::HttpResponse::newHttpResponse();
          resp->setStatusCode(k400BadRequest);
          resp->setBody(R"({"error":"Failed to upload job"})");
          callback(resp);
        }
      }


    void Routes::Quit(const drogon::HttpRequestPtr &request, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
      {
        handleInvalidMethod(request);
        m_running = false;

        auto resp = HttpResponse::newHttpResponse();
        resp->setBody(htmlSetBody("COMET Server shut down.", "/quit", "Servant Shutdown"));
        callback(resp);

        // Wait 5 seconds and then quit the application  on a separate thread
        std::thread([]()
          {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            COMETLOG("WebServices::quit() - Shutting down server now.", LoggerLevel::INFO);
            app().quit();
          }).detach();

        COMETLOG(
          "WebServices::quit() - Shutting down server in 1s to allow serving the images in the callback page.",
          LoggerLevel::INFO);
      }

    std::string Routes::htmlSetBody(const std::string &body, const std::string &targetPath,
                                    const std::string &title)
      {
        return "<!DOCTYPE html>"
               "<html>"
               + htmlHeader(targetPath, title)
               + "<body>" + body + "</body>"
               + htmlFooter()
               + "</html>";
      }


    std::string Routes::htmlHeader(const std::string &fullTargetPath, const std::string &title)
      {
        struct Link
          {
            std::string name;
            std::string href;
            bool showIfNotAuthenticated;
            bool showIfNotManager;
            bool showIfNotDebug;
          };

        std::vector<Link> links = {

          {"Authentication", "/authentication", true, true, true},
          {"Servant Settings", "/servant/settings", false, true, true},
          {"Servant Summary", "/servant/summary", false, false, true},
          {"Job Summary", "/job/summary", false, false, true},
          {"Reset Running Jobs (Dev only)", "/resetrunningjobs/", false, false, false},
          {"Mock Run Jobs (Dev only)", "/mockrunjobs/", false, false, false},
          {"Run (Queued)", "/run_queued/", false, false, false},
          {"Quit", "/quit", false, false, true},
        };

        // Go through all the links, removing if showIfNotAuthenticated, showIfNotManager or showIfNotDebug conditions not met
        bool isAuthenticated = auth.machineAuthenticationisValid();
        bool isManager = aServant.isManager();
#ifdef _DEBUG
        bool isDebug = true;
#else
        bool isDebug = false;
#endif
        // Remove links that should not be shown based on the current state
        links.erase(
          std::remove_if(
            links.begin(), links.end(),
            [&](const Link &link)
              {
                if (!isAuthenticated && !link.showIfNotAuthenticated)
                  return true;
                if (!isManager && !link.showIfNotManager)
                  return true;
                if (!isDebug && !link.showIfNotDebug)
                  return true;
                return false;
              }),
          links.end());


        std::string targetPath = fullTargetPath;
        size_t queryPos = targetPath.find('?');
        if (queryPos != std::string::npos) {
          targetPath = targetPath.substr(0, queryPos);
        }


        std::string linksHTML;
        for (const auto &link: links) {
          std::string linkPath = link.href;
          std::string style = (link.href == targetPath) ? " class='highlight' " : " ";
          if (link.href == targetPath) linkPath = fullTargetPath;
          linksHTML += "<a href='" + linkPath + "' " + style + " >" + link.name + "</a> ";
        }

        if (fullTargetPath == "/quit")
          linksHTML = "";

        std::string datetimestring = std::to_string(std::time(nullptr));
        return R"(
        <head>
            <title>)" + title + R"(</title>
            <link rel="stylesheet" href="/css/styles.css?v=)" + datetimestring + R"(">
            <link rel='shortcut icon' type='image/png' href='/media/COMET_Icon.png'>
            <script src="/js/general.js"></script>
        </head>
        <header>
            <img src='/media/COMET_DarkBG.svg' alt='1' height='60'>
            <h1>)" + title + R"(</h1>
            )" + linksHTML + R"(
            <p></p>
        </header>
        )";
      }

    std::string Routes::htmlFooter()
      {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&now_c), "%d %b %Y at %I:%M:%S %p");

        auto configurationFilePath = db.getDatabasePath();

        return R"(
        <footer style="margin-top: auto; display: flex; justify-content: space-between; align-items: center;">
            <h4 style="margin: 0;">)" + oss.str() + R"(</h4>
            <h4 style="margin: 0;">&copy; )" + std::to_string(1900 + std::localtime(&now_c)->tm_year) +
               R"( COMET Strategy</h4>
            <h4 style="margin: 0;">Config File: )" + configurationFilePath + R"(</h4>
        </footer>
    )";
      }
  }
