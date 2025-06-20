#include <iostream>

#include <drogon/drogon.h>
using namespace drogon;

#include <trantor/utils/Logger.h>
#ifdef _WIN32
#include <ws2tcpip.h>
#else
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

#include <drogon/drogon.h>
using namespace drogon;

int main()
{

    std::filesystem::path sourceDir = std::filesystem::path(__FILE__).parent_path();

    //app().setLogPath(sourceDir.string());
    //app().setLogLevel(trantor::Logger::kWarn);

    // `registerHandler()` adds a handler to the desired path. The handler is
    // responsible for generating a HTTP response upon an HTTP request being
    // sent to Drogon


    // Set log file path
    // app().setLogPath(sourceDir.string())
    //      .setLogLevel(trantor::Logger::kInfo);


    
    app().registerHandler(
        "/",
        [](const HttpRequestPtr &request,
           std::function<void(const HttpResponsePtr &)> &&callback) {
            LOG_INFO << "connected:"
                     << (request->connected() ? "true" : "false");
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Hello from COMET Servants!");
            
            LOG_INFO << "Hello from COMET Servants!";
            callback(resp);
        },
        {Get});

    // `registerHandler()` also supports parsing and passing the path as
    // parameters to the handler. Parameters are specified using {}. The text
    // inside the {} does not correspond to the index of parameter passed to the
    // handler (nor it has any meaning). Instead, it is only to make it easier
    // for users to recognize the function of each parameter.
    app().registerHandler(
        "/user/{user-name}",
        [](const HttpRequestPtr &,
           std::function<void(const HttpResponsePtr &)> &&callback,
           const std::string &name) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Hello COMET USER, " + name + "!");
            callback(resp);
        },
        {Get});

    // You can also specify that the parameter is in the query section of the
    // URL!
    app().registerHandler(
        "/hello?user={user-name}",
        [](const HttpRequestPtr &,
           std::function<void(const HttpResponsePtr &)> &&callback,
           const std::string &name) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setBody("Hello COMET Parameter User, " + name + "!");
            callback(resp);
        },
        {Get});

    // Or, if you want to, instead of asking drogon to parse it for you. You can
    // parse the request yourselves.
    app().registerHandler(
        "/hello_user",
        [](const HttpRequestPtr &req,
           std::function<void(const HttpResponsePtr &)> &&callback) {
            auto resp = HttpResponse::newHttpResponse();
            auto name = req->getOptionalParameter<std::string>("user");
            if (!name)
                resp->setBody("Please tell COMET your name");
            else
                resp->setBody("Hello, " + name.value() + ", from COMET!");
            callback(resp);
        },
        {Get});

    app()
        .setBeforeListenSockOptCallback([](int fd) {
            LOG_INFO << "setBeforeListenSockOptCallback:" << fd;
#ifdef _WIN32
#elif __linux__
            int enable = 1;
            if (setsockopt(
                    fd, IPPROTO_TCP, TCP_FASTOPEN, &enable, sizeof(enable)) ==
                -1)
            {
                LOG_INFO << "setsockopt TCP_FASTOPEN failed";
            }
#else
#endif
        })
        .setAfterAcceptSockOptCallback([](int) {});

    // Ask Drogon to listen on 127.0.0.1 port 8848. Drogon supports listening
    // on multiple IP addresses by adding multiple listeners. For example, if
    // you want the server also listen on 127.0.0.1 port 5555. Just add another
    // line of addListener("127.0.0.1", 5555)
    LOG_INFO << "Server running on 127.0.0.1:8848";
    app().addListener("127.0.0.1", 8848).run();
}
