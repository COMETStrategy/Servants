//
// Created by Brett King on 17/6/2025.
//

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H
#include <string>

namespace comet
    {
        class Authentication {

            private:
                std::string email;
                std::string code;
                std::string machineId;
                bool isAuthenticated;
                int totalCores = 0;

            public:
                void set_total_cores(int total_cores);

                void set_unused_cores(int unused_cores);

                void set_manager_ip_address(const std::string &manager_ip_address);

            private:
                int unusedCores = 0;
                std::string managerIpAddress = "";

            public:
                Authentication();

                bool valid(std::string email, std::string code, std::string machineUrl);
                std::string HtmlAuthenticationForm();

                bool CheckVerificationInformation();

                static std::string getPrivateIPAddress();

                static std::string getPublicIPAddressFromWeb();

                std::string getMachineId();
        };
    
    }



#endif //AUTHENTICATION_H
