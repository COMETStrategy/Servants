//
// Created by Brett King on 5/6/2025.
//

#ifndef WEBSERVICES_H
#define WEBSERVICES_H

#endif //WEBSERVICES_H

#include <string>
#include <thread>
#include <memory>

class WebServices
  {
  public:
    WebServices();
    ~WebServices();

    void initialize();
    void shutdown();
    
    void handleRequest(const std::string& request);
    
    void run();
    void join();
    bool isRunning() const;
  private:
    std::unique_ptr<std::thread> serverThread_;
    std::atomic<bool> running_{true};
  };
