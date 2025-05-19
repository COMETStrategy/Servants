#include <drogon/drogon.h>

void startServer() {
    drogon::app().loadConfigFile("config.json").run();
}
