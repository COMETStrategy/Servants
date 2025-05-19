#ifdef _WIN32
#include <windows.h>
#include <drogon/drogon.h>

void WINAPI ServiceMain(DWORD, LPTSTR*) {
    drogon::app().loadConfigFile("config.json").run();
}
#endif
