#include "Services/WebServices.h"
#include "Services/Logger.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <ctime>
#include <iomanip>
#include <sstream>

int main()
  {
    comet::WebServices server;
    
    server.run();
    comet::Logger::log("Server is running. Press Ctrl+C to stop.", comet::DEBUG);

    server.join();
    comet::Logger::log( std::string("Completed ") );
    
    return 0;
  }
