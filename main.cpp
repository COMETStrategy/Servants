#include <iostream>

int main() {
#ifdef _WIN32
    std::cout << "Run the Windows Service entry point for deployment." << std::endl;
#else
    std::cout << "Starting server..." << std::endl;
    drogon::app().loadConfigFile("config.json").run();
#endif
    return 0;
}
