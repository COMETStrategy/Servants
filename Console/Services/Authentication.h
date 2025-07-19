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
                bool isAuthenticated;


            public:
                Authentication();

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
