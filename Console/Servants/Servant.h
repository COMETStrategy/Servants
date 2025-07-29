//
// Created by Brett King on 26/6/2025.
//

#ifndef SERVANT_H
#define SERVANT_H
#include <string>
#include <map>


namespace Json
  {
    class Value;
  }

namespace comet
  {
    class Authenticator;
    class Database;
    class Servant
      {
      public:
        Servant();

      private:
        int totalCores;
        int unusedCores = 0;
        int activeCores = 0;
        int port;
        std::string email;
        std::string code;
        std::string ipAddress;
        std::string managerIpAddress;
        std::string version;
        std::string engineFolder;
        std::string centralDataFolder;
        int projectId;
        double priority;
        Authenticator *auth;
        bool alive;
      public:
        static Servant *thisServant;
        
      public:
        void setTotalCores(int total_cores);
        void setUnusedCores(int unused_cores);
        void setActiveCores(int active_cores);
        void setPort(int aPort);
        void setProjectId(int aProjectId);
        void setEmail(const std::string &aEmail);
        void setCode(const std::string &aCode);
        void setIpAddress(const std::string &aIpAddress);
        void setManagerIpAddress(const std::string &aManagerIpAddress);
        void setVersion(const std::string &aVersion);
        void setPriority(const double aPriority);
        void setAlive(const int aAlive);

        void setAuthentication(Authenticator *authState);
        

        int getTotalCores() const{return totalCores;}
        int getUnusedCores() const{return unusedCores;}
        int getActiveCores() const { return activeCores; }
        std::string getCode() {return code;}

        void setEngineFolder(const std::string & newFolder);

        std::string getEngineFolder();

        void setCentralDataFolder(const std::string& newFolder);
        std::string getCentralDataFolder();

        static void stopSelectedProcesses(Database& db, Json::Value & jobs);

        static void initialiseAllServantActiveCores(const Database & db);

        static void decrementActiveProcessCount(Database &db, int decrementAmount);
        static int deleteServants(const Database &db, const Json::Value &servants);
        static std::string htmlServantSummary(Database &db);;
        std::string getEmail() {return email;};
        std::string getIpAddress() {return ipAddress;}
        int getPort() const {return port;};
        double getPriority() const {return priority;};
        std::string getManagerIpAddress() {return managerIpAddress;};;
        
        bool isManager();

        std::string htmlAuthenticationSettingsForm(Authenticator &auth);
        std::string htmlServantSettingsForm();

        static bool createNewServentsTable(Database &db);
        void updateServantSettings(Database & db);

        static std::vector<std::map<std::string, std::string>> getAvailableServants(Database &db);

        static std::vector<std::map<std::string, std::string>> getAllServants(Database &db);

        static void checkAllServantsAlive(Database &db);



      public:
        // UpdateStatus on managerIP
        bool routineStatusUpdates() const;

        void startRoutineStatusUpdates();

        void updateDatabase(Database &db);
        void updateManagerDatabase();
      };
    
  };


#endif //SERVANT_H
