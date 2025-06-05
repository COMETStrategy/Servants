#include "WebServices.h"
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <thread>
#include <chrono>
#include <iostream>

using namespace drogon;

WebServices::WebServices()
{
    std::cout << "WebServices::WebServices()" << std::endl;
    initialize();
}

WebServices::~WebServices()
{
    shutdown();
    if (serverThread_ && serverThread_->joinable()) serverThread_->join();

    std::cout << "WebServices::~WebServices()" << std::endl;
}

void WebServices::initialize()
{
    std::cout << "WebServices::initialize()" << std::endl;

    // Register handlers
    app().registerHandler(
        "/",
        [](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            LOG_INFO << "connected:" << (request->connected() ? "true" : "false");
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Hello from COMET Servants!");
            LOG_INFO << "Hello from COMET Servants!";
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
           [this](const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback) {
               running_ = false; // Set running to false
               auto resp = HttpResponse::newHttpResponse();
               resp->setBody("Server is shutting down...");
               callback(resp);
               std::thread([] {
                   std::this_thread::sleep_for(std::chrono::milliseconds(100));
                   std::exit(0);
               }).detach();
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
    std::cout << "WebServices::shutdown()" << std::endl;
}

void WebServices::handleRequest(const std::string& request)
{
    std::cout << "WebServices::handleRequest() - Request: " << request << std::endl;
}

void WebServices::run()
    {
        serverThread_ = std::make_unique<std::thread>([] {
            LOG_INFO << "Server running on 127.0.0.1:8848";
            app().addListener("127.0.0.1", 8848).run();
        });
    }

void WebServices::join()
    {
        if (serverThread_ && serverThread_->joinable())
            serverThread_->join();
    }

bool WebServices::isRunning() const
    {
        return running_;
    }