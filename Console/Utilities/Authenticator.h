//
// Created by Brett King on 17/6/2025.
//

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H
#include <string>

namespace comet
    {
        class Authenticator {

            private:
                bool isAuthenticated;


            public:
                Authenticator();

                bool valid(std::string email, std::string code, std::string machineUrl);
                std::string HtmlAuthenticationForm();

                bool CheckVerificationInformation(std::string email, std::string code);
                bool machineAuthenticationisValid() const
                    {
                        return isAuthenticated;
                    }



 
        };
    
    }



#endif //AUTHENTICATION_H
