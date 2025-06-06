
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>

#include "WebServices.h"
#include "Logger.h"
#include "Job.h"

using namespace std;
using namespace drogon;
using namespace comet;

WebServices::WebServices()
  {
    comet::Logger::setLoggerLevel(LoggerLevel::ALL);
    comet::Logger::log("WebServices::WebServices()", LoggerLevel::DEBUG);
    m_port = 7777;;
    initialize();
  }

WebServices::WebServices(unsigned short port)
  {
    m_port = port;
    initialize();
  }

WebServices::~WebServices()
  {
    shutdown();
    if (m_serverThread && m_serverThread->joinable()) m_serverThread->join();

    comet::Logger::log("WebServices::~WebServices()", LoggerLevel::DEBUG);
  }

void WebServices::initialize()
  {
    comet::Logger::log("WebServices::initialize()", LoggerLevel::DEBUG);

    // Register handlers
    app().registerHandler(
      "/",
      [this](const HttpRequestPtr &request,
             std::function<void(const HttpResponsePtr &)> &&callback)
        {
           auto resp = HttpResponse::newHttpResponse();
          resp->setBody(setHTMLBody("COMET Servant are alive! (Get /)"));
          Logger::log("COMET Servants get / is alive!");
          callback(resp);
        },
      {Get});
    app().registerHandler(
      "/",
      [this](const HttpRequestPtr &request,
             std::function<void(const HttpResponsePtr &)> &&callback)
        {
          auto resp = HttpResponse::newHttpResponse();
          resp->setBody(R"({
                            "company": "COMET Strategy - Australia",
                            "HubName": "COMET Strategy - Australia (C++ Servants):LOCAL",
                            "HubAddress": "http://localhost:7777/",
                            "CloudDataConnection": "",
                            "ErrorMessage": ""
                          })");
          Logger::log("COMET Servants post alive!");
          callback(resp);
        },
      {Post});

    app().registerHandler(
  "/upload/job/",
  [this](const HttpRequestPtr &request,
         std::function<void(const HttpResponsePtr &)> &&callback) {
    // Parse the JSON payload
    auto json = request->getJsonObject();
    if (!json) {
      // Respond with an error if the JSON is invalid
      auto resp = HttpResponse::newHttpResponse();
      resp->setStatusCode(k400BadRequest);
      resp->setBody(R"({"error":"Invalid JSON payload"})");
      callback(resp);
      return;
    }

    // Process the JSON data
    uploadJob(request, *json);

    // Respond with a success message
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k200OK);
    resp->setBody(R"({"status":"Data received successfully"})");
    callback(resp);
  },
  {Post});


    app().registerHandler(
      "/quit",
      [this](const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          m_running = false; // Set running to false

          auto resp = HttpResponse::newHttpResponse();
          resp->setBody(setHTMLBody("Server is shutting down..."));
          callback(resp);

          // Stop the server from listening for new connections
          app().quit();
          Logger::log("WebServices::quit() - Shutting down server...", LoggerLevel::DEBUG);

          // std::thread([]
          //   {
          //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
          //     std::exit(0);
          //   }).detach();
        },
      {Get});

    app().setBeforeListenSockOptCallback([](int fd)
          {
#ifdef _WIN32
            // Windows-specific options
#elif __linux__
        int enable = 1;
        if (setsockopt(
                fd, IPPROTO_TCP, TCP_FASTOPEN, &enable, sizeof(enable)) == -1)
        {
            LOG_INFO << "setsockopt TCP_FASTOPEN failed";
        }
#else
            // Other platforms
#endif
          })
        .setAfterAcceptSockOptCallback([](int)
          {
            // Optional: code after accept
          });
  }

void WebServices::shutdown()
  {
    comet::Logger::log("WebServices::shutdown()", LoggerLevel::DEBUG);
  }

void WebServices::handleRequest(const std::string &request)
  {
    comet::Logger::log(std::string("WebServices::handleRequest() - Request: ") + request);
  }

std::string WebServices::setHTMLBody(const std::string &body) const
  {
    return R"(
          <!DOCTYPE html>
          <html>
          )" + getHTMLHeader() + R"(
          <body>
          <div style="margin: 1cm;">
          )" + body + R"(
          </div>
          </body>
          )" + getHTMLFooter() + R"(
          </html>
          )";
  }

std::string WebServices::getHTMLHeader() const
  {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&now_c);

    std::ostringstream oss;
    oss << (1900 + localTime->tm_year); // tm_year is years since 1900

    return R"(
    <link rel="stylesheet" href="https://support.cometstrategy.com/themes/zCSS.css">
    <link rel='shortcut icon' type='image/png' href='https://media.cometstrategy.com/img/COMET_Icon.png'>
    <header>
    <h1>
    <img src='https://media.cometstrategy.com/img/COMET_DarkBG.svg' alt='1' height='60'>
    <span class='heading_title' COMET Servants</span>
    
    Welcome to COMET Servants</h1> 
    <h3>(c) )" + oss.str() + R"( COMET Strategy</h3>
    
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
    <footer style="margin-top: auto;">
    
    <h4>Run on )" + oss.str() + R"( </h4>
    
    </footer>

           )";
  }

void WebServices::run()
  {
    m_serverThread = std::make_unique<std::thread>([this]
      {
        Logger::log(std::string("Server running on 127.0.0.1:") + to_string(m_port));
        app().addListener("127.0.0.1", m_port).run();
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

void WebServices::uploadJob(const HttpRequestPtr &request, const Json::Value &json) {
    // Extract headers
    auto emailHeader = request->getHeader("X-Email");
    auto codeHeader = request->getHeader("X-Code");

    Job job (emailHeader, codeHeader, json);

 
}