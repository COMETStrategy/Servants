//
// Created by Brett King on 17/6/2025.
//

#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H
#include <string>

namespace comet
    {
        class Authentication {

            public:
                static bool registerServant(std::string email, std::string code, std::string machineUrl);
                static std::string HtmlAuthenticationForm();

                static std::string getPrivateIPAddress();

                static std::string getPublicIPAddressFromWeb();

        };
    
    }



#endif //AUTHENTICATION_H
