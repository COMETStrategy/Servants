//
// Created by Brett King on 26/6/2025.
//

#ifndef SERVANT_H
#define SERVANT_H
#include <string>


namespace comet
  {
    class Authentication;
    class Servant
      {
      public:
        Servant();

      private:
        int totalCores;
        int unusedCores = 0;
        int activeCores = 0;
        std::string managerIpAddress;
        Authentication *auth;
      public:
        void set_total_cores(int total_cores);

        void set_unused_cores(int unused_cores);

        void set_manager_ip_address(const std::string &manager_ip_address);

        std::string HtmlAuthenticationForm();

        [[nodiscard]] int getTotalCores() const{return totalCores;}
        [[nodiscard]] int getUnusedCores() const{return unusedCores;}
        [[nodiscard]] int getActiveCores() const { return activeCores; }

        void setAuthentication(Authentication *authState);

      public:
        // UpdateStatus on managerIP
        bool routineStatusUpdates(const Authentication &auth) const;

        void startRoutineStatusUpdates(const Authentication &auth);
      };
    
  };


#endif //SERVANT_H
