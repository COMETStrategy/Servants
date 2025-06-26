//
// Created by Brett King on 26/6/2025.
//

#ifndef SERVANT_H
#define SERVANT_H
#include <string>

namespace comet
  {
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
        int projectId;
        Authentication *auth;
        
      public:
        void set_totalCores(int total_cores);
        void set_unusedCores(int unused_cores);
        void set_activeCores(int active_cores);
        void set_port(int aPort);
        void set_projectId(int aProjectId);
        void set_email(const std::string &aEmail);
        void set_code(const std::string &aCode);
        void set_ipAddress(const std::string &aIpAddress);
        void set_managerIpAddress(const std::string &aManagerIpAddress);
        void set_version(const std::string &aVersion);
        std::string get_ipAddress() {return ipAddress;}

        void updateServantSettings(Database & db);

        std::string get_code();

        std::string get_email();;
        int get_port() const {return port;};

        std::string HtmlAuthenticationSettingsForm(Authentication &auth);
        std::string HtmlServantSettingsForm();

        static bool createNewServentsTable(Database &db);

        [[nodiscard]] int getTotalCores() const{return totalCores;}
        [[nodiscard]] int getUnusedCores() const{return unusedCores;}
        [[nodiscard]] int getActiveCores() const { return activeCores; }

        void setAuthentication(Authentication *authState);

      public:
        // UpdateStatus on managerIP
        bool routineStatusUpdates() const;

        void startRoutineStatusUpdates();

        void updateDatabase(Database &db);
      };
    
  };


#endif //SERVANT_H
