
#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>

#include "WebServices/WebServices.h"

#include <drogon/drogon.h>
using namespace drogon;

#include <trantor/utils/Logger.h>
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

int main()
{
    WebServices server;
    std::filesystem::path sourceDir = std::filesystem::path(__FILE__).parent_path();

    // Example: set log file path and level
    // app().setLogPath(sourceDir.string())
    //      .setLogLevel(trantor::Logger::kInfo);

    // Root handler
    drogon::app().registerHandler(
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

    // Handler with path parameter
    drogon::app().registerHandler(
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

    // Handler with query parameter
    drogon::app().registerHandler(
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

    // Handler parsing query manually
    drogon::app().registerHandler(
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

    // /quit handler to shut down the server
    drogon::app().registerHandler(
        "/quit",
        [](const HttpRequestPtr &,
           std::function<void(const HttpResponsePtr &)> &&callback)
        {
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Server is shutting down...");
            callback(resp);
            std::thread([] {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::exit(0);
            }).detach();
        },
        {Get});

    // Optional: set socket options before listening
    drogon::app().setBeforeListenSockOptCallback([](int fd)
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

    LOG_INFO << "Server running on 127.0.0.1:8848";
    drogon::app().addListener("127.0.0.1", 8848).run();
}
