#include "Services/WebServices.h"
#include "Services/Logger.h"

int main()
  {

    try {
     
      comet::WebServices server("~/comet-servants.db");
    
      server.run();
      comet::Logger::log("Server is running. Press Ctrl+C to stop.", comet::DEBUG);
  
      server.join();
    }
    catch (const std::exception& e) {
      comet::Logger::log(std::string("Exception occurred: ") + e.what(), comet::CRITICAL);
      return 1; // Return a non-zero value to indicate failure
    }
    catch (...) {
      comet::Logger::log("An unknown error occurred.", comet::CRITICAL);
      return 1; // Return a non-zero value to indicate failure
    }
    comet::Logger::log( std::string("☑️Completed 👋") );
    
    return 0;
  }
