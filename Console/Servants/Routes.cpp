//
// Created by Brett King on 29/7/2025.
//


#include "Routes.h"

#include "../Utilities//Logger.h"
#include <nlohmann/json.hpp>

using namespace drogon;

namespace comet
  {
    // Initialise static members
    Authenticator Routes::auth;
    Servant Routes::aServant;
    Database Routes::db;
    bool Routes::m_running = true;

    
    void Routes::handleInvalidMethod(const HttpRequestPtr &request)
      {
        std::string upperMethod = to_string(request->method());
        std::transform(upperMethod.begin(), upperMethod.end(), upperMethod.begin(), ::toupper);
        // Get the request body
        //std::string body = std::string(request->getBody());

        COMETLOG("Handling " + upperMethod + " request to " + request->path(), LoggerLevel::DEBUGGING);
      }

    Routes::Routes(Authenticator &anAuth, Servant & servant, Database &database, bool &running)
      {
        auth = anAuth;
        aServant = servant;
        db = database;
        m_running = running;
       
        registerAllHandlers(); 
      }
   

    using vHandler = std::function<void(const drogon::HttpRequestPtr &,
                                        std::function<void(const drogon::HttpResponsePtr &)> &&)>;

    struct PossibleRoutes
      {
        bool validForUnauthorisedAccess;
        bool validForDebugOnly;
        std::string path;
        vHandler handler;
        const std::vector<internal::HttpConstraint> methods;

        PossibleRoutes(bool unauth, bool debug, std::string p,
                       vHandler h,
                       const std::vector<internal::HttpConstraint> &m)
          : validForUnauthorisedAccess(unauth),
            validForDebugOnly(debug),
            path(p),
            handler(h),
            methods(m)
          {
          }
      };

    void Routes::registerAllHandlers()
      {

        std::vector<PossibleRoutes> allPossibleRoutes = {
          {true, true, "/", Routes::Alive, {drogon::Get, drogon::Post}},
          {true, true, "/alive", Routes::Alive, {drogon::Get, drogon::Post}},
          {true, true, "/quit", Routes::Quit, {drogon::Get, drogon::Post}},
          {true, true, "/authenticate", Routes::Authenticate, {drogon::Get, drogon::Post}},
          {false, false, "/authentication", Routes::Authentication, {drogon::Get, drogon::Post}},
          // If you want to register both, add them separately without random
        };

        for (const auto &r: allPossibleRoutes) {
          auto chosen = r.handler;

          drogon::app().registerHandler(r.path, [r](const drogon::HttpRequestPtr &req,
                                                    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
                                          {
                                            r.handler(req, std::move(callback));
                                          }, r.methods);

          COMETLOG(
            std::string("Registering route: ") + r.path + ", Unauthorised:" + std::to_string(r.validForUnauthorisedAccess) +
            ", Debug:" + std::to_string(r.validForDebugOnly), LoggerLevel::INFO);
        }


        //app().registerHandler("/alive", &Routes::Alive, {Get, Post});
      }

    void Routes::Alive(
        const drogon::HttpRequestPtr& request,
        std::function<void(const drogon::HttpResponsePtr&)>&& callback)
      {
              handleInvalidMethod(request);
              if (request->method() == drogon::Post) {
              auto host = request->getHeader("Host");
              std::string hubAddress = "http://" + aServant.getIpAddress() + ":" + std::to_string(aServant.getPort()) + "/";
              nlohmann::json jsonResponse = {
                {"company", "COMET Strategy - Australia"},
                {"HubName", "COMET Servant (" + aServant.getIpAddress()  + "):LOCAL"},
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
                ,""
                ,"Test COMET Servant Alive" )
                );
              COMETLOG("Proof of life: " + request->getHeader("Host") + " is Alive", LoggerLevel::DEBUGGING);
              callback(resp);
      }

    void Routes::Authenticate(const drogon::HttpRequestPtr& request, std::function<void(const drogon::HttpResponsePtr&)>&& callback)
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
              resp->addHeader("Location", "/servant_settings");
              COMETLOG(
                std::string("✅ Authentication ") + ((isAuthenticated) ? "successful" : "failed") + " for email: " +
                aServant
                .getEmail() +
                ". Redirecting to /",
                LoggerLevel::INFO);
              callback(resp);
            
      }

    
    void Routes::Authentication(const drogon::HttpRequestPtr &request,std::function<void(const drogon::HttpResponsePtr &)> &&callback)
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

    
    

    void Routes::Quit(const drogon::HttpRequestPtr &request,                      std::function<void(const drogon::HttpResponsePtr &)> &&callback)
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
          {"Servant Settings", "/servant_settings", false, true, true},
          {"Servant Summary", "/servant_summary", false, false, true},
          {"Job Summary", "/job_summary", false, false, true},
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
