#include <drogon/drogon.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <json/json.h>

#include "WebServices.h"

#include "../Utilities/Authenticator.h"
#include "../Utilities/Database.h"
#include "../Utilities/Logger.h"
#include "../Utilities/Encoding.h"
#include "../Utilities/utilities.h"
#include "Scheduler.h"
#include "Job.h"
#include "Servant.h"
#include "Routes.h"

using namespace std;
using namespace drogon;

namespace comet
  {
    comet::Servant comet::WebServices::aServant;

    WebServices::WebServices(const std::string &dbFilename)
      : db(dbFilename)
      {
                Scheduler::setAutoStartJobs(true);
        configurationFilePath = getFullFilenameAndDirectory(dbFilename);

        createNewDatabase();

        // COMETLOG it
        COMETLOG("Configuration path: " + configurationFilePath, comet::LoggerLevel::INFO);
        comet::Logger::setLoggerLevel(LoggerLevel::INFO);
        COMETLOG("WebServices::WebServices()", LoggerLevel::DEBUGGING);
        aServant.setPort(777);
      
        Routes route(auth, aServant, db, m_running);
        run();

        auto results = db.
            getQueryResults("SELECT * FROM servants where LOWER(iPAddress) = '" + aServant.getIpAddress() + "';");
        // COMETLOG the created Date and the last updated date
        if (!results.empty()) {
          aServant.setTotalCores(stoi(results[0].at("totalCores")));
          aServant.setUnusedCores(stoi(results[0].at("unusedCores")));
          aServant.setActiveCores(stoi(results[0].at("activeCores")));
          aServant.setManagerIpAddress(results[0].at("managerIpAddress"));
          aServant.setEngineFolder(results[0].at("engineFolder"));
          aServant.setCentralDataFolder(results[0].at("centralDataFolder"));
          aServant.setEmail(results[0].at("email"));
          aServant.setCode(results[0].at("code"));
          aServant.setPort(stoi(results[0].at("port")));;
          aServant.setPriority(stod(results[0].at("priority")));;
          //aServant.setIpAddress(results[0].at("ipAddress"));
          for (const auto &row: results) {
            COMETLOG(
              "Database Servant: Registration Date: " + row.at("registrationTime") + ", Last Updated Date: " + row.at(
                "lastUpdateTime"), LoggerLevel::INFO);
          }

          auto email = results[0].at("email");
          auto code = results[0].at("code");
          auto ipAddress = results[0].at("ipAddress");
          if (!auth.valid(email, code, ipAddress)) {
            COMETLOG("Invalid verification code for machine.", LoggerLevel::WARNING);
          }

          aServant.setAuthentication(&auth);
          aServant.startRoutineStatusUpdates();

          Servant::checkAllServantsAlive(db);
          Servant::initialiseAllServantActiveCores(db);
        } else {
          COMETLOG("No records found in Servant Settings table, authentication required.", LoggerLevel::WARNING);
        }
      }


    WebServices::~WebServices()
      {
        shutdown();
        if (m_serverThread && m_serverThread->joinable()) m_serverThread->join();

        COMETLOG("WebServices::~WebServices()", LoggerLevel::DEBUGGING);
      }

    

    

    void WebServices::shutdown()
      {
        COMETLOG("WebServices::shutdown()", LoggerLevel::DEBUGGING);
      }

    void WebServices::handleRequest(const std::string &request)
      {
        COMETLOG(std::string("WebServices::handleRequest() - Request: ") + request, LoggerLevel::INFO);
      }

    std::string WebServices::htmlSetBody(const std::string &body, const std::string &targetPath,
                                         const std::string &title) const
      {
        return "<!DOCTYPE html>"
               "<html>"
               + htmlHeader(targetPath, title)
               + "<body>" + body + "</body>"
               + htmlFooter()
               + "</html>";
      }

    void WebServices::run()
      {
        app().setDocumentRoot("static");

        m_serverThread = std::make_unique<std::thread>([this]
          {
            COMETLOG(std::string("Server running on ") + getMachineName() + ":" + to_string(aServant.getPort())
                     + " (" + getPrivateIPAddress() + ":" + to_string(aServant.getPort())+ ")"
                     , comet::INFO);

            app().addListener("0.0.0.0", aServant.getPort()).run();
          });
      }

    void WebServices::join()
      {
        if (m_serverThread && m_serverThread->joinable())
          m_serverThread->join();
      }

    bool WebServices::isRunning() const
      {
        return m_running;
      }

    std::string WebServices::generateTimestamp()
      {
        // Generate timestamp in the format YYYYMMDDHHMMSS
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm *localTime = std::localtime(&now_c);
        std::ostringstream oss;
        oss << std::put_time(localTime, "%Y%m%d%H%M%S");
        return oss.str();
      }

    bool WebServices::uploadJob(const drogon::HttpRequestPtr &request)
      {
        Job aJob(request, JobStatus::Queued, db);
        return aJob.validJobStatus();
      }

    bool WebServices::createNewDatabase()
      {
        COMETLOG("Database file " + configurationFilePath + " Checking for all tables.",
                 LoggerLevel::DEBUGGING);

        aServant.createNewServentsTable(db);


        Job::createNewJobsTable(db);


        COMETLOG("Servants and Jobs tables created successfully.", LoggerLevel::INFO);
        return true;
      }
    std::string WebServices::htmlFooter() 
    {
        return Routes::htmlFooter();
      
    }
    std::string WebServices::htmlHeader(const std::string &fullTargetPath, const std::string &title) 
      {
        
        return Routes::htmlHeader(fullTargetPath, title);
        
      }

    std::string WebServices::hashIV(const std::string &salt) const
      {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        std::string combined = secret_iv + salt;
        SHA256(reinterpret_cast<const unsigned char *>(combined.c_str()), combined.size(), hash);
        return std::string(reinterpret_cast<const char *>(hash), 16); // Use first 16 bytes
      }

    std::string WebServices::simpleEncrypt(const std::string &simpleString, const std::string &salt) const
      {
        std::string iv = hashIV(salt);
        std::vector<unsigned char> encrypted(simpleString.size() + EVP_MAX_BLOCK_LENGTH);
        int encryptedLength = 0;

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create encryption context");

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                               reinterpret_cast<const unsigned char *>(secret_key.c_str()),
                               reinterpret_cast<const unsigned char *>(iv.c_str())) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to initialize encryption");
        }

        if (EVP_EncryptUpdate(ctx, encrypted.data(), &encryptedLength,
                              reinterpret_cast<const unsigned char *>(simpleString.c_str()),
                              simpleString.size()) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to encrypt data");
        }

        int finalLength = 0;
        if (EVP_EncryptFinal_ex(ctx, encrypted.data() + encryptedLength, &finalLength) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to finalize encryption");
        }

        encryptedLength += finalLength;
        EVP_CIPHER_CTX_free(ctx);

        return std::string(reinterpret_cast<const char *>(encrypted.data()), encryptedLength);
      }

    std::string WebServices::simpleDecrypt(const std::string &encryptedString, const std::string &salt) const
      {
        std::string iv = hashIV(salt);
        std::vector<unsigned char> decrypted(encryptedString.size());
        int decryptedLength = 0;

        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Failed to create decryption context");

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                               reinterpret_cast<const unsigned char *>(secret_key.c_str()),
                               reinterpret_cast<const unsigned char *>(iv.c_str())) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to initialize decryption");
        }

        if (EVP_DecryptUpdate(ctx, decrypted.data(), &decryptedLength,
                              reinterpret_cast<const unsigned char *>(encryptedString.c_str()),
                              encryptedString.size()) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to decrypt data");
        }

        int finalLength = 0;
        if (EVP_DecryptFinal_ex(ctx, decrypted.data() + decryptedLength, &finalLength) != 1) {
          EVP_CIPHER_CTX_free(ctx);
          throw std::runtime_error("Failed to finalize decryption");
        }

        decryptedLength += finalLength;
        EVP_CIPHER_CTX_free(ctx);

        return std::string(reinterpret_cast<const char *>(decrypted.data()), decryptedLength);
      }

    std::string WebServices::utf8Encode(const std::string &input)
      {
        // Assuming the input is already UTF-8 encoded
        return input;
      }

    std::string WebServices::base64Decode(const std::string &encoded)
      {
        static const std::string base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        std::string output;
        std::vector<int> decoding_table(256, -1);
        for (int i = 0; i < 64; i++) {
          decoding_table[base64_chars[i]] = i;
        }

        int val = 0, valb = -8;
        for (unsigned char c: encoded) {
          if (decoding_table[c] == -1) break;
          val = (val << 6) + decoding_table[c];
          valb += 6;
          if (valb >= 0) {
            output.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
          }
        }
        return output;
      }
  }
