#include <drogon/drogon.h>

void connectToDatabase() {
    auto dbClient = drogon::app().getDbClient();
    dbClient->execSqlAsync("SELECT NOW()", [](const drogon::orm::Result &r) {
        LOG_INFO << "Current time: " << r[0]["now"].as<std::string>();
    }, [](const drogon::orm::DrogonDbException &e) {
        LOG_ERROR << e.base().what();
    });
}
