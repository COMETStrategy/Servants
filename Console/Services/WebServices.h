//
// Created by Brett King on 5/6/2025.
//

#ifndef WEBSERVICES_H
#define WEBSERVICES_H
#include "Authentication.h"
#include "drogon/HttpRequest.h"
#include "drogon/utils/FunctionTraits.h"

#endif //WEBSERVICES_H

#include <string>
#include <thread>
#include <memory>
#include <openssl/sha.h>
#include "Database.h"

namespace comet
  {
    class WebServices
      {
      public:
        WebServices(const std::string& dbFilename = "~/comet-servants.db");
        ~WebServices();

        void initializeHandlers();
        void shutdown();
    
        void handleRequest(const std::string& request);
        std::string setHTMLBody(const std::string& body) const;
        std::string getHTMLHeader() const;
        std::string getHTMLFooter() const;
    
        void run();
        void join();
        bool isRunning() const;

        std::string generateTimestamp();

        bool uploadJob(const drogon::HttpRequestPtr &request);

      private:
        std::unique_ptr<std::thread> m_serverThread;
        std::atomic<bool> m_running{true};
        unsigned short m_port;
        Database db;
        Authentication auth;

        const std::string ciphering = "aes-256-cbc"; // Cipher method
        const std::string secret_key = "your_secret_key"; // Replace with your secret key
        const std::string secret_iv = "your_secret_iv";   // Replace with your secret IV
        std::string hashIV(const std::string &salt) const;
        std::string simpleEncrypt(const std::string &simpleString, const std::string &salt) const;
        std::string simpleDecrypt(const std::string &simpleString, const std::string &salt) const;
        std::string utf8Encode(const std::string &input);
        std::string base64Decode(const std::string &encoded);

      };
  }
