//
// Created by Brett King on 5/6/2025.
//

#ifndef WEBSERVICES_H
#define WEBSERVICES_H


#include <string>
#include <thread>
#include <memory>
#include "drogon/utils/FunctionTraits.h"
#include "Database.h"
#include "Authentication.h"
#include "Scheduler.h"
#include "Servant.h"


namespace comet
  {

    class WebServices
      {
      public:
        explicit WebServices(const std::string& dbFilename = "~/comet-servants.db");
        ~WebServices();

        void registerResetRunningJobsHandler();

        void registerUpdateAliveServantsHandler();

        void registerMockRunJobsHandler();

        void shutdown();

        void handleRequest(const std::string& request);
        std::string setHTMLBody(const std::string& body, const std::string &targetPath, const std::string &title) const;
        std::string getHTMLHeader(const std::string &targetPath, const std::string &title) const;
        std::string getHTMLFooter() const;

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
        std::atomic<bool> m_running{true};
        Database db;
        Authentication auth;
        std::string configurationFilePath; // Default database path
        static comet::Servant aServant;
        Scheduler scheduler;
        
      public:

        void initializeHandlers();
        static void handleInvalidMethod(const drogon::HttpRequestPtr &request);
      private:
        void registerRootAuthenticationHandler();
        void registerAuthenticateHandler();
        void registerConfigurationHandler();
        void registerUploadJobHandler();
        void registerRootJobSummaryHandler();
        void registerJobStartHandler();

        void registerJobStatusDatabaseUpdateHandler();

        void registerServantSummaryHandler();

        void registerServantStatusHandler();
        void registerStatusHandler();
        void registerStatusJobsHandler();
        void registerQuitHandler();
        

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