#include <drogon/drogon.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <json/json.h>

#include "WebServices.h"

#include "Authentication.h"
#include "Database.h"
#include "Logger.h"
#include "Job.h"
#include "Encoding.h"
#include "Scheduler.h"
#include "utilities.h"
#include "Servant.h"
#include "../../../../../../opt/homebrew/Cellar/c-ares/1.34.4/include/ares.h"

using namespace std;
using namespace drogon;

namespace comet
  {
    comet::Servant comet::WebServices::aServant;

    WebServices::WebServices(const std::string &dbFilename)
      : db(dbFilename)
      {
        configurationFilePath = getFullFilenameAndDirectory(dbFilename);

        createNewDatabase();

        // COMETLOG it
        COMETLOG("Configuration path: " + configurationFilePath, comet::LoggerLevel::INFO);
        comet::Logger::setLoggerLevel(LoggerLevel::INFO);
        COMETLOG("WebServices::WebServices()", LoggerLevel::DEBUGGING);
        aServant.setPort(7777);
        initializeHandlers();
        run();

        auto results = db.
            getQueryResults("SELECT * FROM servants where iPAddress = '" + aServant.getIpAddress() + "';");
        // COMETLOG the created Date and the last updated date
        if (!results.empty()) {
          aServant.setTotalCores(stoi(results[0].at("totalCores")));
          aServant.setUnusedCores(stoi(results[0].at("unusedCores")));
          aServant.setActiveCores(stoi(results[0].at("activeCores")));
          aServant.setManagerIpAddress(results[0].at("managerIpAddress"));
          aServant.setEmail(results[0].at("email"));
          aServant.setCode(results[0].at("code"));
          aServant.setPort(stoi(results[0].at("port")));;
          aServant.setPriority(stod(results[0].at("priority")));;
          aServant.setIpAddress(results[0].at("ipAddress"));
          for (const auto &row: results) {
            COMETLOG(
              "Database Servant: Registration Date: " + row.at("registrationTime") + ", Last Updated Date: " + row.at(
                "lastUpdateTime"), LoggerLevel::INFO);
          }

          auto email = results[0].at("email");
          auto code = results[0].at("code");
          auto ipAddress = results[0].at("ipAddress");
          if (!auth.valid(email, code, ipAddress)) {
            COMETLOG("Invalid verification code for machine.", LoggerLevel::WARNING);
          }

          aServant.setAuthentication(&auth);
          aServant.startRoutineStatusUpdates();
        } else {
          COMETLOG("No records found in Servant Settings table, authentication required.", LoggerLevel::WARNING);
        }
      }


    WebServices::~WebServices()
      {
        shutdown();
        if (m_serverThread && m_serverThread->joinable()) m_serverThread->join();

        COMETLOG("WebServices::~WebServices()", LoggerLevel::DEBUGGING);
      }

    void WebServices::initializeHandlers()
      {
        COMETLOG("WebServices::initialize()", LoggerLevel::DEBUGGING);

        registerRootHandler();
        registerAuthenticateHandler();
        registerConfigurationHandler();
        registerJobStartHandler();
        registerJobStatusDatabaseUpdateHandler();
        registerJobSummaryHandler();
        registerMockRunJobsHandler();
        registerResetRunningJobsHandler();
        registerServantStatusHandler();
        registerServantSummaryHandler();
        registerStatusHandler();
        registerStatusJobsHandler();
        registerUpdateAliveServantsHandler();
        registerUploadJobHandler();
        registerQuitHandler();
      }

    void WebServices::handleInvalidMethod(const HttpRequestPtr &request)
      {
        std::string upperMethod = to_string(request->method());
        std::transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), ::toupper);
        COMETLOG("Handling " + upperMethod + " request to " + request->path(), LoggerLevel::INFO);
      }

    void WebServices::registerRootHandler()
      {
        app().registerHandler(
          "/",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
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
              std::string responseBody = aServant.HtmlAuthenticationSettingsForm(auth);
              if (auth.machineAuthenticationisValid()) responseBody += aServant.HtmlServantSettingsForm();
              resp->setBody(setHTMLBody(responseBody, "/"));
              COMETLOG("COMET Servant Home: alive!", LoggerLevel::INFO);
              callback(resp);
            },
          {Get, Post});
      }

    void WebServices::registerAuthenticateHandler()
      {
        app().registerHandler(
          "/authenticate",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);

              aServant.setEmail(request->getParameter("email"));
              aServant.setCode(request->getParameter("code"));
              aServant.setIpAddress(request->getParameter("ipAddress"));

              const bool isAuthenticated = auth.valid(aServant.getEmail(), aServant.getCode(), aServant.getIpAddress());
              if (!isAuthenticated) {
                COMETLOG("Invalid authentication parameters", LoggerLevel::CRITICAL);
              }

              aServant.updateServantSettings(db);

              auto resp = HttpResponse::newHttpResponse();
              resp->setStatusCode(k302Found);
              resp->addHeader("Location", "/");
              COMETLOG(
                std::string("✅ Authentication ") + ((isAuthenticated) ? "successful" : "failed") + " for email: " +
                aServant
                .getEmail() +
                ". Redirecting to /",
                LoggerLevel::INFO);
              callback(resp);
            },
          {Post});
      }

    void WebServices::registerConfigurationHandler()
      {
        app().registerHandler(
          "/configuration",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
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
              auto alive = request->getParameter("alive");
              if (alive.empty()) alive = "0";

              aServant.setTotalCores(std::stoi(totalCores));
              aServant.setUnusedCores(std::stoi(unusedCores));
              aServant.setActiveCores(std::stoi(activeCores));
              aServant.setPriority(std::stod(priority));
              aServant.setManagerIpAddress(managerIpAddress);
              aServant.setAlive(std::stoi(alive));

              if (!db.insertRecord(
                "UPDATE servants SET totalCores = '" + totalCores + "' "
                ", unusedCores = '" + unusedCores + +"' "
                ", activeCores = '" + activeCores + +"' "
                ", managerIpAddress = '" + managerIpAddress + "' "
                ", lastUpdateTime = DATETIME('now') "
                "WHERE ipAddress = '" + aServant.getIpAddress() + "';")) {
                COMETLOG("Failed to update Settings table with configuration information: ",
                         LoggerLevel::CRITICAL);

                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->addHeader("Location", "/");
                resp->setBody(R"({"error":"Failed to update Settings table with configuration information"})");
                callback(resp);
                return;
              }

              aServant.updateDatabase(db);

              auto resp = HttpResponse::newHttpResponse();
              resp->setStatusCode(k302Found);
              resp->addHeader("Location", "/");
              COMETLOG("Configuration successfully saved. Redirecting to /", LoggerLevel::INFO);
              callback(resp);
            },
          {Post});
      }

    void WebServices::registerMockRunJobsHandler()
      {
        app().registerHandler(
          "/mockrunjobs/",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);

              try {
                int updatedRows = Job::mockRunJobs(db);

                nlohmann::json responseJson = {
                  {"ErrorMessage", ""},
                  {"Status", "Mock run jobs (" + std::to_string(updatedRows) + " row/s affected)."},
                  {"Message", "Mock run jobs executed successfully"},
                  {"UpdatedRows", updatedRows}
                };

                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k302Found); // Set status code to 302 for redirection
                resp->addHeader("Location", "/Job_summary"); // Redirect to /Job_summary
                callback(resp);

                COMETLOG("Successfully executed mock run jobs", LoggerLevel::INFO);
              } catch (const std::exception &e) {
                COMETLOG(std::string("Error executing mock run jobs: ") + e.what(), LoggerLevel::CRITICAL);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(R"({"ErrorMessage":"Failed to execute mock run jobs"})");
                callback(resp);
              }
            },
          {Post, Get});
      }

    void WebServices::registerResetRunningJobsHandler()
      {
        app().registerHandler(
          "/resetrunningjobs/",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);


              try {
                // Reset all jobs with status "Running" to "Queued"
                int updatedRows = Job::resetRunningJobs(db);

                nlohmann::json responseJson = {
                  {"ErrorMessage", ""},
                  {
                    "Status",
                    "Reset Allocated and Running to received status (" + to_string(updatedRows) + " rows affected)."
                  },
                  {"Message", "Reset running jobs to queued"},
                  {"UpdatedRows", updatedRows}
                };

                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k302Found); // Set status code to 302 for redirection
                resp->addHeader("Location", "/Job_summary"); // Redirect to /Job_summary
                callback(resp);

                COMETLOG("Successfully reset running jobs to queued", LoggerLevel::INFO);
              } catch (const std::exception &e) {
                COMETLOG(std::string("Error resetting running jobs: ") + e.what(), LoggerLevel::CRITICAL);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(R"({"ErrorMessage":"Failed to reset running jobs"})");
                callback(resp);
              }
            },
          {Post, Get});
      }

    void WebServices::registerUpdateAliveServantsHandler()
      {
        app().registerHandler(
          "/updateAliveServants",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);
              if (!auth.machineAuthenticationisValid()) {
                COMETLOG("Unauthorized access to /updateAliveServants", LoggerLevel::WARNING);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k401Unauthorized);
                resp->setBody("Unauthorized");
                callback(resp);
                return;
              }

              
              auto resp = HttpResponse::newHttpResponse();
              resp->setStatusCode(k302Found); // Set status code to 302 for redirection
              resp->addHeader("Location", "/servant_summary"); 
              resp->setBody("Checking status of Servants and then will redirect to Servant summary..."); 

              callback(resp);

              
              Servant::checkAllServantsAlive(db);
              

            },
          {Post, Get});
      }

    void WebServices::registerUploadJobHandler()
      {
        app().registerHandler(
          "/upload/job/",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);
              if (!aServant.isManager()) {
                COMETLOG(
                  "This Servant does not accept jobs since it is not the managing Servant, submit to " + aServant.
                  getManagerIpAddress(),
                  LoggerLevel::DEBUGGING);
              }

              if (!db.isConnected()) {
                COMETLOG("Database connection failed", LoggerLevel::CRITICAL);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k406NotAcceptable);
                resp->setBody(R"({"ErrorMessage":"Error mySQL database connection"})");
                callback(resp);
                return;
              }

              auto json = request->getJsonObject();
              if (!json) {
                COMETLOG("Invalid JSON payload", LoggerLevel::WARNING);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"error":"Invalid JSON payload"})");
                callback(resp);
                return;
              }

              if (uploadJob(request)) {
                auto newJobsStarted = Scheduler::startJobsOnBestServants(db);
                auto resp = HttpResponse::newHttpResponse();
                // Start the job if possible
                resp->setStatusCode(k201Created);
                resp->setBody(R"({"status":"Job uploaded successfully"})");
                COMETLOG("Response sent: Job uploaded successfully", LoggerLevel::DEBUGGING);
                callback(resp);
              } else {
                COMETLOG("Failed to upload job", LoggerLevel::WARNING);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"error":"Failed to upload job"})");
                callback(resp);
              }
            },
          {Post});
      }


    void WebServices::registerJobStartHandler()
      {
        app().registerHandler(
          "/start/job/",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);

              if (!aServant.isManager()) {
                COMETLOG(
                  "This Servant does not start jobs since it is not the managing Servant, submit to " + aServant.
                  getManagerIpAddress(),
                  LoggerLevel::DEBUGGING);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k403Forbidden);
                resp->setBody(R"({"ErrorMessage":"This servant is not authorized to start jobs"})");
                callback(resp);
                return;
              }

              auto job = request->getJsonObject();
              if (!job) {
                COMETLOG("Invalid JSON payload", LoggerLevel::WARNING);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"ErrorMessage":"Invalid JSON job payload"})");
                callback(resp);
                return;
              }

              try {
                nlohmann::json jobData;
                for (const auto &key: job->getMemberNames()) {
                  jobData[key] = (*job)[key].asString(); // Adjust type conversion as needed
                }
                bool jobStarted = Job::startJob(db, jobData);

                if (jobStarted) {
                  auto resp = HttpResponse::newHttpResponse();
                  resp->setStatusCode(k200OK);
                  resp->setBody(R"({"status":"Job started successfully"})");
                  COMETLOG("Job started successfully ", LoggerLevel::INFO);
                  callback(resp);
                } else {
                  auto resp = HttpResponse::newHttpResponse();
                  resp->setStatusCode(k400BadRequest);
                  resp->setBody(R"({"ErrorMessage":"Failed to start job"})");
                  COMETLOG("Failed to start job", LoggerLevel::WARNING);
                  callback(resp);
                }
              } catch (const std::exception &e) {
                COMETLOG(std::string("Error starting job: ") + e.what(), LoggerLevel::CRITICAL);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k500InternalServerError);
                resp->setBody(R"({"ErrorMessage":"Internal server error"})");
                callback(resp);
              }
            },
          {Post});
      }

    void WebServices::registerJobStatusDatabaseUpdateHandler()
      {
        app().registerHandler(
          "/job/status/database/update",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
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
            },
          {Post});
      }

    void WebServices::registerJobSummaryHandler()
      {
        app().registerHandler(
          "/job_summary",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);
              if (!auth.machineAuthenticationisValid()) {
                COMETLOG("Unauthorized access to /job_summary", LoggerLevel::WARNING);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k401Unauthorized);
                resp->setBody("Unauthorized");
                callback(resp);
                return;
              }

              auto sort = request->getParameter("sort");
              auto filter = request->getParameter("filter");

              std::string report = Job::jobSummaryHtmlReport(db, sort, filter);

              auto resp = HttpResponse::newHttpResponse();
              resp->setBody(setHTMLBody(report, "/job_summary"));
              callback(resp);
            },
          {Get});
      }

    void WebServices::registerServantSummaryHandler()
      {
        app().registerHandler(
          "/servant_summary",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);
              if (!auth.machineAuthenticationisValid()) {
                COMETLOG("Unauthorized access to /servant_summary", LoggerLevel::WARNING);
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k401Unauthorized);
                resp->setBody("Unauthorized");
                callback(resp);
                return;
              }


              std::string report = Servant::servantSummaryHtmlReport(db);

              auto resp = HttpResponse::newHttpResponse();
              resp->setBody(setHTMLBody(report, "/servant_summary"));
              callback(resp);
            },
          {Get});
      }

    void WebServices::registerServantStatusHandler()
      {
        app().registerHandler(
          "/servant/status",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);

              auto json = request->getJsonObject();
              if (!json) {
                auto resp = HttpResponse::newHttpResponse();
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
                aServant.setIpAddress((*json)["ipAddress"].asString());
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
            },
          {Post});
      }

    void WebServices::registerStatusHandler()
      {
        app().registerHandler(
          "/status/",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);

              auto email = request->getHeader("X-Email");
              auto code = request->getHeader("X-Code");

              if (email.empty() || code.empty()) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"ErrorMessage":"Missing required headers"})");
                callback(resp);
                return;
              }

              auto json = request->getJsonObject();
              if (!json || !json->isMember("CloudDataConnection")) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"ErrorMessage":"Invalid or missing JSON payload"})");
                callback(resp);
                return;
              }

              std::string cloudDataConnection = (*json)["CloudDataConnection"].asString();
              COMETLOG("Received CloudDataConnection: " + cloudDataConnection, LoggerLevel::INFO);

              nlohmann::json responseJson = {
                {"Status", "success"},
                {"message", "Status updated successfully"},
                {"errorMessage", ""}
              };

              auto resp = HttpResponse::newHttpResponse();
              resp->setStatusCode(k200OK);
              resp->setContentTypeCode(CT_APPLICATION_JSON);
              resp->setBody(responseJson.dump());
              callback(resp);
            },
          {Post});
      }

    void WebServices::registerStatusJobsHandler()
      {
        app().registerHandler(
          "/status/jobs/",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);

              // Get GroupName from Posted data
              auto json = request->getJsonObject();
              if (!json || !json->isMember("GroupName")) {
                auto resp = HttpResponse::newHttpResponse();
                resp->setStatusCode(k400BadRequest);
                resp->setBody(R"({"ErrorMessage":"Invalid or missing GroupName in JSON payload"})");
                callback(resp);
                return;
              }
              std::string GroupName = (*json)["GroupName"].asString();
              std::string jobStatuses = Job::getAllJobStatuses(db, GroupName);

              jobStatuses = R"({"ErrorMessage": "", "Status": ")" + jobStatuses + R"("})";
              auto resp = HttpResponse::newHttpResponse();
              resp->addHeader("Access-Control-Allow-Headers", "Content-type");
              resp->setContentTypeCode(CT_TEXT_HTML);
              resp->setBody(jobStatuses);
              callback(resp);
            },
          {Get, Post});
      }

    void WebServices::registerQuitHandler()
      {
        app().registerHandler(
          "/quit",
          [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
            {
              handleInvalidMethod(request);
              m_running = false;

              auto resp = HttpResponse::newHttpResponse();
              resp->setBody(setHTMLBody("Server is shutting down...", "/"));
              callback(resp);

              app().quit();
              COMETLOG("WebServices::quit() - Shutting down server...", LoggerLevel::DEBUGGING);
            },
          {Get});
      }


    void WebServices::shutdown()
      {
        COMETLOG("WebServices::shutdown()", LoggerLevel::DEBUGGING);
      }

    void WebServices::handleRequest(const std::string &request)
      {
        COMETLOG(std::string("WebServices::handleRequest() - Request: ") + request, LoggerLevel::INFO);
      }

    std::string WebServices::setHTMLBody(const std::string &body, const std::string &targetPath) const
      {
        return "<!DOCTYPE html>"
        "<html>"
        + getHTMLHeader(targetPath)
        + "<body>" + body + "</body>"
        +  getHTMLFooter()
        + "</html>";
      }

    std::string WebServices::getHTMLHeader(const std::string &targetPath) const
      {
        struct Link
          {
            std::string name;
            std::string href;
          };

        std::vector<Link> links = {
          //{"Public", "https://www.cometstrategy.com/"},
          //{"Support", "https://support.cometstrategy.com/?site=support&page=dashboard"},
          {"Job Summary", "/job_summary?sort=date&filter=all"},
          {"Servant Summary", "/servant_summary"},
          {"Reset Running Jobs (Dev only)", "/resetrunningjobs/"},
          {"Mock Run Jobs (Dev only)", "/mockrunjobs/"},
          {"Settings", "/"},
          {"Quit", "/quit"}
        };

        std::string linksHTML;
        for (const auto &link: links) {
          std::string style = (link.href == targetPath)
                                ? "style='font-weight: bold; margin-right: 10px'"
                                : "";
          linksHTML += "<a href='" + link.href + "' class='system_link' " + style + " style='margin-right: 10px;'>" +
              link.
              name + "</a> ";
        }

        std::string datetimestring = std::to_string(std::time(nullptr));
        return R"(
    <link rel="stylesheet" href="/css/styles.css?v=)" + datetimestring + R"(">
    <link rel='shortcut icon' type='image/png' href='/media/COMET_Icon.png'>
    <header>
        <img src='/media/COMET_DarkBG.svg' alt='1' height='60'>
        <span class='heading_title'>COMET Servants</span>
        <h1>Welcome to COMET Servants</h1>
        )" + linksHTML + R"(
    </header>
    )";
      }

    std::string WebServices::getHTMLFooter() const
      {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&now_c), "%d %b %Y at %I:%M:%S %p");

        return R"(
        <footer style="margin-top: auto; display: flex; justify-content: space-between; align-items: center;">
            <h4 style="margin: 0;">)" + oss.str() + R"(</h4>
            <h4 style="margin: 0;">&copy; )" + std::to_string(1900 + std::localtime(&now_c)->tm_year) +
               R"( COMET Strategy</h4>
            <h4 style="margin: 0;">Config File: )" + configurationFilePath + R"(</h4>
        </footer>
    )";
      }

    void WebServices::run()
      {
        app().setDocumentRoot("static");

        m_serverThread = std::make_unique<std::thread>([this]
          {
            COMETLOG(std::string("Server running on localhost:") + to_string(aServant.getPort())
                     + ", Private local IP: " + getPrivateIPAddress() + ":" + to_string(aServant.getPort())
                     + " or Public IP: " + getPublicIPAddressFromWeb() + ":" + to_string(aServant.getPort())
                     , comet::INFO);

            app().addListener("0.0.0.0", aServant.getPort()).run();
          });
      }

    void WebServices::join()
      {
        if (m_serverThread && m_serverThread->joinable())
          m_serverThread->join();
      }

    bool WebServices::isRunning() const
      {
        return m_running;
      }

    std::string WebServices::generateTimestamp()
      {
        // Generate timestamp in the format YYYYMMDDHHMMSS
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm *localTime = std::localtime(&now_c);
        std::ostringstream oss;
        oss << std::put_time(localTime, "%Y%m%d%H%M%S");
        return oss.str();
      }

    bool WebServices::uploadJob(const drogon::HttpRequestPtr &request)
      {
        Job aJob(request, JobStatus::Queued, db);
        return aJob.validJobStatus();
      }

    bool WebServices::createNewDatabase()
      {
        COMETLOG("Database file " + configurationFilePath + " Checking for all tables.",
                 LoggerLevel::INFO);

        aServant.createNewServentsTable(db);


        Job::createNewJobsTable(db);


        COMETLOG("Servants and Jobs tables created successfully.", LoggerLevel::INFO);
        return true;
      }


    std::string WebServices::hashIV(const std::string &salt) const
      {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        std::string combined = secret_iv + salt;
        SHA256(reinterpret_cast<const unsigned char *>(combined.c_str()), combined.size(), hash);
        return std::string(reinterpret_cast<const char *>(hash), 16); // Use first 16 bytes
      }

    std::string WebServices::simpleEncrypt(const std::string &simpleString, const std::string &salt) const
      {
        std::string iv = hashIV(salt);
        std::vector<unsigned char> encrypted(simpleString.size() + EVP_MAX_BLOCK_LENGTH);
        int encryptedLength = 0;

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create encryption context");

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                               reinterpret_cast<const unsigned char *>(secret_key.c_str()),
                               reinterpret_cast<const unsigned char *>(iv.c_str())) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to initialize encryption");
        }

        if (EVP_EncryptUpdate(ctx, encrypted.data(), &encryptedLength,
                              reinterpret_cast<const unsigned char *>(simpleString.c_str()),
                              simpleString.size()) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to encrypt data");
        }

        int finalLength = 0;
        if (EVP_EncryptFinal_ex(ctx, encrypted.data() + encryptedLength, &finalLength) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to finalize encryption");
        }

        encryptedLength += finalLength;
        EVP_CIPHER_CTX_free(ctx);

        return std::string(reinterpret_cast<const char *>(encrypted.data()), encryptedLength);
      }

    std::string WebServices::simpleDecrypt(const std::string &encryptedString, const std::string &salt) const
      {
        std::string iv = hashIV(salt);
        std::vector<unsigned char> decrypted(encryptedString.size());
        int decryptedLength = 0;

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create decryption context");

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                               reinterpret_cast<const unsigned char *>(secret_key.c_str()),
                               reinterpret_cast<const unsigned char *>(iv.c_str())) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to initialize decryption");
        }

        if (EVP_DecryptUpdate(ctx, decrypted.data(), &decryptedLength,
                              reinterpret_cast<const unsigned char *>(encryptedString.c_str()),
                              encryptedString.size()) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to decrypt data");
        }

        int finalLength = 0;
        if (EVP_DecryptFinal_ex(ctx, decrypted.data() + decryptedLength, &finalLength) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to finalize decryption");
        }

        decryptedLength += finalLength;
        EVP_CIPHER_CTX_free(ctx);

        return std::string(reinterpret_cast<const char *>(decrypted.data()), decryptedLength);
      }

    std::string WebServices::utf8Encode(const std::string &input)
      {
        // Assuming the input is already UTF-8 encoded
        return input;
      }

    std::string WebServices::base64Decode(const std::string &encoded)
      {
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        std::string output;
        std::vector<int> decoding_table(256, -1);
        for (int i = 0; i < 64; i++) {
          decoding_table[base64_chars[i]] = i;
        }

        int val = 0, valb = -8;
        for (unsigned char c: encoded) {
          if (decoding_table[c] == -1) break;
          val = (val << 6) + decoding_table[c];
          valb += 6;
          if (valb >= 0) {
            output.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
          }
        }
        return output;
      }
  }
