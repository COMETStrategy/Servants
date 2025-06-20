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

            public:
                Authentication();

                bool validate(std::string email, std::string code, std::string machineUrl);
                std::string HtmlAuthenticationForm();

                bool CheckVerificationInformation();

                static std::string getPrivateIPAddress();

                static std::string getPublicIPAddressFromWeb();

                std::string getMachineId();
        };
    
    }



#endif //AUTHENTICATION_H
