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

                bool valid(std::string email, std::string code, std::string machineUrl);
                std::string HtmlAuthenticationForm();

                bool CheckVerificationInformation();
                bool machineAuthenticationisValid() const
                    {
                        return isAuthenticated;
                    }

            public:
                [[nodiscard]] std::string getEmail() const
                    {
                        return email;
                    }

                [[nodiscard]] std::string getCode() const
                    {
                        return code;
                    }

 
        };
    
    }



#endif //AUTHENTICATION_H
