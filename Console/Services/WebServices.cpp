#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

#include "WebServices.h"

#include "Authentication.h"
#include "Database.h"
#include "Logger.h"
#include "Job.h"
#include "Encoding.h"

using namespace std;
using namespace drogon;
using namespace comet;

WebServices::WebServices(const std::string &dbFilename)
  : db(dbFilename)
  {
    comet::Logger::setLoggerLevel(LoggerLevel::INFO);
    comet::Logger::log("WebServices::WebServices()", LoggerLevel::DEBUG);
    m_port = 7777;;
    initializeHandlers();

    auto results = db.getQueryResults("SELECT * FROM version;");
    // Log the created Date and the last updated date
    if (!results.empty()) {
      for (const auto &row: results) {
        comet::Logger::log(
          "Database Version: Created Date: " + row.at("createdDate") + ", Last Updated Date: " + row.at(
            "lastUpdatedDate"));
      }
    } else {
      comet::Logger::log("No records found in version table.", LoggerLevel::WARNING);
    }
  }


WebServices::~WebServices()
  {
    shutdown();
    if (m_serverThread && m_serverThread->joinable()) m_serverThread->join();

    comet::Logger::log("WebServices::~WebServices()", LoggerLevel::DEBUG);
  }

void WebServices::initializeHandlers()
  {
    comet::Logger::log("WebServices::initialize()", LoggerLevel::DEBUG);

    // Register handlers
    app().registerHandler(
      "/",
      [this](const HttpRequestPtr &request,
             std::function<void(const HttpResponsePtr &)> &&callback)
        {
          auto resp = HttpResponse::newHttpResponse();
          std::string responseBody = "Welcome to COMET Servants!";
          responseBody += Authentication::HtmlAuthenticationForm();
          resp->setBody(setHTMLBody(responseBody));
          Logger::log("COMET Servants alive!");
          callback(resp);
        },
      {Get});

    app().registerHandler(
      "/upload/job/",
      [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          comet::Logger::log("Handling POST request to /upload/job/", LoggerLevel::DEBUG);

          // Database connection
          if (!db.isConnected()) {
            comet::Logger::log("Database connection failed", LoggerLevel::CRITICAL);
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k406NotAcceptable);
            resp->setBody(R"({"ErrorMessage":"Error mySQL database connection"})");
            callback(resp);
            return;
          }
          // Parse the JSON payload
          auto json = request->getJsonObject();
          if (!json) {
            comet::Logger::log("Invalid JSON payload", LoggerLevel::WARNING);
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            resp->setBody(R"({"error":"Invalid JSON payload"})");
            callback(resp);
            return;
          }
          if (uploadJob(request)) {
            // Successfully uploaded job
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k201Created);
            resp->setBody(R"({"status":"Job uploaded successfully"})");
            comet::Logger::log("Response sent: Job uploaded successfully", LoggerLevel::DEBUG);
            callback(resp);
          } else {
            comet::Logger::log("Failed to upload job", LoggerLevel::WARNING);
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k400BadRequest);
            resp->setBody(R"({"error":"Failed to upload job"})");
            callback(resp);
            return;
          }

        },
      {Post});


    app().registerHandler(
      "/quit",
      [this](const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          m_running = false; // Set running to false

          auto resp = HttpResponse::newHttpResponse();
          resp->setBody(setHTMLBody("Server is shutting down..."));
          callback(resp);

          // Stop the server from listening for new connections
          app().quit();
          Logger::log("WebServices::quit() - Shutting down server...", LoggerLevel::DEBUG);
        },
      {Get});

    app().setBeforeListenSockOptCallback([](int fd)
          {
#ifdef _WIN32
            // Windows-specific options
#elif __linux__
        int enable = 1;
        if (setsockopt(
                fd, IPPROTO_TCP, TCP_FASTOPEN, &enable, sizeof(enable)) == -1)
        {
            LOG_INFO << "setsockopt TCP_FASTOPEN failed";
        }
#else
            // Other platforms
#endif
          })
        .setAfterAcceptSockOptCallback([](int)
          {
            // Optional: code after accept
          });
  }

void WebServices::shutdown()
  {
    comet::Logger::log("WebServices::shutdown()", LoggerLevel::DEBUG);
  }

void WebServices::handleRequest(const std::string &request)
  {
    comet::Logger::log(std::string("WebServices::handleRequest() - Request: ") + request);
  }

std::string WebServices::setHTMLBody(const std::string &body) const
  {
    return R"(
          <!DOCTYPE html>
          <html>
          )" + getHTMLHeader() + R"(
          <body>
          <div style="margin: 1cm;">
          )" + body + R"(
          </div>
          </body>
          )" + getHTMLFooter() + R"(
          </html>
          )";
  }

std::string WebServices::getHTMLHeader() const
  {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm *localTime = std::localtime(&now_c);

    std::ostringstream oss;
    oss << (1900 + localTime->tm_year); // tm_year is years since 1900

    return R"(
    <link rel="stylesheet" href="https://support.cometstrategy.com/themes/zCSS.css">
    <link rel='shortcut icon' type='image/png' href='https://media.cometstrategy.com/img/COMET_Icon.png'>
    <header>
    <h1>
    <img src='https://media.cometstrategy.com/img/COMET_DarkBG.svg' alt='1' height='60'>
    <span class='heading_title' COMET Servants</span>
    
    Welcome to COMET Servants</h1> 
    <h3>(c) )" + oss.str() + R"( COMET Strategy</h3>
    
    </header>

           )";
  }

std::string WebServices::getHTMLFooter() const
  {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_c), "%d %b %Y at %I:%M:%S %p");

    return R"(
    <footer style="margin-top: auto;">
    
    <h4>Run on )" + oss.str() + R"( </h4>
    
    </footer>

           )";
  }

void WebServices::run()
  {
    m_serverThread = std::make_unique<std::thread>([this]
      {
        Logger::log(std::string("Server running on 127.0.0.1:") + to_string(m_port)
          + " or Private local IP: " + comet::Authentication::getPrivateIPAddress() + ":" + to_string(m_port)
          , comet::INFO);

       app().addListener("0.0.0.0", m_port).run();
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
    Job *job = new Job(request);
    job->setJobStatus("Queued");
    auto query = job->getReplaceQueryString();

    comet::Logger::log("Executing query: " + query, LoggerLevel::DEBUG);
    auto result = db.executeQuery(query);
    if (!result) {
      comet::Logger::log("Failed to execute database query: " + query, LoggerLevel::CRITICAL);
      return false; // Or handle the error appropriately
    }

    comet::Logger::log("✅ " + job->description());
    delete job; // Clean up the job object

    return true;
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
