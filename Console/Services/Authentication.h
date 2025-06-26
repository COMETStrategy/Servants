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



            private:
                std::string code;
                std::string machineId;
                bool isAuthenticated;
                int totalCores;
                int unusedCores = 0;
                int activeCores = 0;
                std::string managerIpAddress;

            public:
                void set_total_cores(int total_cores);
                void set_unused_cores(int unused_cores);
                void set_manager_ip_address(const std::string &manager_ip_address);

            private:

            public:
                Authentication();

                bool valid(std::string email, std::string code, std::string machineUrl);
                std::string HtmlAuthenticationForm();

                bool CheckVerificationInformation();

            public:
                [[nodiscard]] std::string getEmail() const
                    {
                        return email;
                    }

                [[nodiscard]] std::string getCode() const
                    {
                        return code;
                    }

                [[nodiscard]] int getTotalCores() const
                    {
                        return totalCores;
                    }

                [[nodiscard]] int getUnusedCores() const
                    {
                        return unusedCores;
                    }

        };
    
    }



#endif //AUTHENTICATION_H
