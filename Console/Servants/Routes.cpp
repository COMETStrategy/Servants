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

    Routes::Routes(Servant &servant, Database &database, bool &running)
      {
        aServant = servant;
        db = database;
        m_running = running;
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
              resp->setBody("Proof of life: " + request->getHeader("Host") + " is Alive.. :-)\n Go to: " + request->getHeader("Host") + "/authenticate");
              COMETLOG("Proof of life: " + request->getHeader("Host") + " is Alive", LoggerLevel::DEBUGGING);
              callback(resp);
      }


    void Routes::Quit(const drogon::HttpRequestPtr &request,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback)
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
               "<body>" + body + "</body>"
               "</html>";
        // return "<!DOCTYPE html>"
        //   "<html>"
        //   + htmlHeader(targetPath, title)
        //   + "<body>" + body + "</body>"
        //   + htmlFooter()
        //   + "</html>";
      }
  }
