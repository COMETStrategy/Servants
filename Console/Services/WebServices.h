//
// Created by Brett King on 5/6/2025.
//

#ifndef WEBSERVICES_H
#define WEBSERVICES_H

#endif //WEBSERVICES_H

#include <string>
#include <thread>
#include <memory>
namespace comet
  {
    class WebServices
      {
      public:
        WebServices();
        WebServices(unsigned short port);
        ~WebServices();

        void initialize();
        void shutdown();
    
        void handleRequest(const std::string& request);
        std::string setHTMLBody(const std::string& body) const;
        std::string getHTMLHeader() const;
        std::string getHTMLFooter() const;
    
        void run();
        void join();
        bool isRunning() const;
      private:
        std::unique_ptr<std::thread> m_serverThread;
        std::atomic<bool> m_running{true};
        unsigned short m_port;
      };
  }
