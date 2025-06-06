#include "WebServices.h"
#include "Logger.h"
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace std;
using namespace drogon;
using namespace comet;

WebServices::WebServices()
  {
    comet::Logger::setLoggerLevel(LoggerLevel::ALL);
    comet::Logger::log( "WebServices::WebServices()");
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

    comet::Logger::log( "WebServices::~WebServices()");
  }

void WebServices::initialize()
  {
    comet::Logger::log( "WebServices::initialize()");

    // Register handlers
    app().registerHandler(
      "/",
      [](const HttpRequestPtr &request,
         std::function<void(const HttpResponsePtr &)> &&callback)
        {
          comet::Logger::log( string("connected:") + (request->connected() ? "true" : "false"));
          auto resp = HttpResponse::newHttpResponse();
          resp->setBody("Hello from COMET Servants!");
          Logger::log(  "Hello from COMET Servants!");
          callback(resp);
        },
      {Get});

    app().registerHandler(
      "/user/{user-name}",
      [](const HttpRequestPtr &,
         std::function<void(const HttpResponsePtr &)> &&callback,
         const std::string &name)
        {
          auto resp = HttpResponse::newHttpResponse();
          resp->setBody("Hello COMET USER, " + name + "!");
          callback(resp);
        },
      {Get});

    app().registerHandler(
      "/hello?user={user-name}",
      [](const HttpRequestPtr &,
         std::function<void(const HttpResponsePtr &)> &&callback,
         const std::string &name)
        {
          auto resp = HttpResponse::newHttpResponse();
          resp->setBody("Hello COMET Parameter User, " + name + "!");
          callback(resp);
        },
      {Get});

    app().registerHandler(
      "/hello_user",
      [](const HttpRequestPtr &req,
         std::function<void(const HttpResponsePtr &)> &&callback)
        {
          auto resp = HttpResponse::newHttpResponse();
          auto name = req->getOptionalParameter<std::string>("user");
          if (!name)
            resp->setBody("Please tell COMET your name");
          else
            resp->setBody("Hello, " + name.value() + ", from COMET!");
          callback(resp);
        },
      {Get});

    app().registerHandler(
      "/quit",
      [this](const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          m_running = false; // Set running to false
          
          auto resp = HttpResponse::newHttpResponse();
          resp->setBody("Server is shutting down...");
          callback(resp);

          // Stop the server from listening for new connections
          app().quit();
          Logger::log( "WebServices::quit() - Shutting down server...", LoggerLevel::CRITICAL);

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
    comet::Logger::log( "WebServices::shutdown()", LoggerLevel::CRITICAL);
  }

void WebServices::handleRequest(const std::string &request)
  {
    comet::Logger::log( std::string("WebServices::handleRequest() - Request: ") + request ) ;
  }

void WebServices::run()
  {
    m_serverThread = std::make_unique<std::thread>([this]
      {
        Logger::log(  "Server running on 127.0.0.1:" + m_port, LoggerLevel::CRITICAL);
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
