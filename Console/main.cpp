#include "Services/WebServices.h"
#include "Services/Logger.h"

int main()
  {

    try {
     
      comet::WebServices server("~/comet-servants.db");
      
      COMETLOG("Server is running. Press Ctrl+C to stop.", comet::DEBUGGING);
  
      server.join();
    }
    catch (const std::exception& e) {
      COMETLOG(std::string("Exception occurred: ") + e.what(), comet::CRITICAL);
      return 1; // Return a non-zero value to indicate failure
    }
    catch (...) {
      COMETLOG("An unknown error occurred.", comet::CRITICAL);
      return 1; // Return a non-zero value to indicate failure
    }
    COMETLOG( std::string("☑️Completed 👋"), comet::INFO );
    
    return 0;
  }
