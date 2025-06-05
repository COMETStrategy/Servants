#include "WebServices/WebServices.h"
#include <iostream>
#include <thread>
#include <chrono>

#include <ctime>
#include <iomanip>

int main()
    {
        WebServices server;
        server.run();

        // Running in main
        // Print the time every 5 seconds
        std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;
        // timer is 10 seconds
        const auto timerInterval = std::chrono::seconds(5);
            auto startTime = std::chrono::system_clock::now();
        while (server.isRunning())
            {
            
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

            std::cout << "Server running at " << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S")
                      << " | Uptime: " << elapsed << " seconds" << std::endl;
            
            std::this_thread::sleep_for(timerInterval);
            
            }
        
        
        server.join();
        return 0;
    }