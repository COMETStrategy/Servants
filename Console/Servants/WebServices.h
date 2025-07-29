//
// Created by Brett King on 5/6/2025.
//

#ifndef WEBSERVICES_H
#define WEBSERVICES_H


#include <string>
#include <thread>
#include <memory>
#include <drogon/drogon.h>
#include "../Utilities/Database.h"
#include "../Utilities/Authenticator.h"
#include "Scheduler.h"
#include "Servant.h"


namespace comet
  {

    class WebServices
      {
      public:
        explicit WebServices(const std::string& dbFilename = "~/comet-servants.db");
        ~WebServices();

        void shutdown();

        void handleRequest(const std::string& request);
        std::string htmlSetBody(const std::string& body, const std::string &targetPath, const std::string &title) const;
        
        static std::string htmlHeader(const std::string &fullTargetPath, const std::string &title);
        static std::string htmlFooter();

        void run();
        void join();
        bool isRunning() const;

        std::string generateTimestamp();

        bool uploadJob(const drogon::HttpRequestPtr &request);
        bool createNewDatabase();


        static Servant &getServant()
          {
            return aServant;
          }

      private:
        std::unique_ptr<std::thread> m_serverThread;
        bool m_running = true; // Flag to indicate if the server is running
        Database db;
        Authenticator auth;
        std::string configurationFilePath; // Default database path
        static comet::Servant aServant;
        Scheduler scheduler;
        
      public:

           

        const std::string ciphering = "aes-256-cbc"; // Cipher method
        const std::string secret_key = "comet_servant_secret_key"; // Replace with your secret key
        const std::string secret_iv = "comet_servant_secret_iv";   // Replace with your secret IV
        std::string hashIV(const std::string &salt) const;
        std::string simpleEncrypt(const std::string &simpleString, const std::string &salt) const;
        std::string simpleDecrypt(const std::string &simpleString, const std::string &salt) const;
        std::string utf8Encode(const std::string &input);
        std::string base64Decode(const std::string &encoded);
      };
  }

#endif // WEBSERVICES_H