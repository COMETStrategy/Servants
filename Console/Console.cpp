// Console.cpp : Defines the entry point for the application.
//

#include "Utilities/Logger.h"
#include "Servants/WebServices.h"

#include "Console.h"


int main()
{
  #if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
  #endif
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
  COMETLOG(std::string("☑️Completed 👋"), comet::INFO);

  return 0;
}
