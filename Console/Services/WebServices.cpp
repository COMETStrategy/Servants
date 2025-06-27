#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <thread>
#include <chrono>
#include <ctime>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <nlohmann/json.hpp>

#include "WebServices.h"

#include "Authentication.h"
#include "Database.h"
#include "Logger.h"
#include "Job.h"
#include "Encoding.h"
#include "utilities.h"
#include "Servant.h"

using namespace std;
using namespace drogon;
using namespace comet;

WebServices::WebServices(const std::string &dbFilename)
  : db(dbFilename)
  {
    configurationFilePath = getFullFilenameAndDirectory(dbFilename);

    createNewDatabase();

    // Log it
    comet::Logger::log("Configuration path: " + configurationFilePath, comet::LoggerLevel::INFO);
    comet::Logger::setLoggerLevel(LoggerLevel::INFO);
    comet::Logger::log("WebServices::WebServices()", LoggerLevel::DEBUG);
    aServant.setPort(7777);
    initializeHandlers();
    run();

    auto results = db.getQueryResults("SELECT * FROM servants where iPAddress = '" + aServant.getIpAddress() + "';");
    // Log the created Date and the last updated date
    if (!results.empty()) {
      aServant.setTotalCores(stoi(results[0].at("totalCores")));
      aServant.setUnusedCores(stoi(results[0].at("unusedCores")));
      aServant.setActiveCores(stoi(results[0].at("activeCores")));
      aServant.setManagerIpAddress(results[0].at("managerIpAddress"));
      aServant.setEmail(results[0].at("email"));
      aServant.setCode(results[0].at("code"));
      aServant.setPort(stoi(results[0].at("port")));;
      aServant.setIpAddress(results[0].at("ipAddress"));
      for (const auto &row: results) {
        comet::Logger::log(
          "Database Servant: Registration Date: " + row.at("registrationTime") + ", Last Updated Date: " + row.at(
            "lastUpdateTime"));
      }

      auto email = results[0].at("email");
      auto code = results[0].at("code");
      auto ipAddress = results[0].at("ipAddress");
      if (!auth.valid(email, code, ipAddress)) {
        comet::Logger::log("Invalid verification code for machine.", LoggerLevel::WARNING);
      }

      aServant.setAuthentication(&auth);
      aServant.startRoutineStatusUpdates();
    } else {
      comet::Logger::log("No records found in Settings table, authentication required.", LoggerLevel::WARNING);
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
          std::string responseBody = aServant.HtmlAuthenticationSettingsForm(auth);
          if (auth.machineAuthenticationisValid()) responseBody += aServant.HtmlServantSettingsForm();
          resp->setBody(setHTMLBody(responseBody, "/"));
          Logger::log("COMET Servant Home: alive!");
          callback(resp);
        },
      {Get});

    app().registerHandler(
      "/authenticate",
      [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          comet::Logger::log("Handling POST request to /authenticate", LoggerLevel::DEBUG);

          // Parse the JSON payload
          // Extract form parameters
          aServant.setEmail(request->getParameter("email"));
          aServant.setCode(request->getParameter("code"));
          aServant.setIpAddress(request->getParameter("ipAddress"));

          // Update Validation
          const bool isAuthenticated = auth.valid(aServant.getEmail(), aServant.getCode(), aServant.getIpAddress());
          if (!isAuthenticated) {
            comet::Logger::log("Invalid authentication parameters", LoggerLevel::CRITICAL);
          }

          aServant.updateServantSettings(db);

          // Perform authentication (replace with your logic)
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k302Found);
          resp->addHeader("Location", "/");
          comet::Logger::log(
            std::string("✅ Authentication ") + ((isAuthenticated) ? "successful" : "failed") + " for email: " + aServant
            .getEmail() +
            ". Redirecting to /",
            LoggerLevel::INFO);
          callback(resp);
        },
      {Post});

    app().registerHandler(
      "/configuration",
      [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          comet::Logger::log("Handling POST request to /configuration", LoggerLevel::DEBUG);

          // Extract form parameters
          auto totalCores = request->getParameter("totalCores");
          auto unusedCores = request->getParameter("unusedCores");
          if (unusedCores.empty()) unusedCores = "0";
          auto activeCores = request->getParameter("activeCores");
          if (activeCores.empty()) activeCores = "0";
          auto managerIpAddress = request->getParameter("managerIpAddress");

          aServant.setTotalCores(std::stoi(totalCores));
          aServant.setUnusedCores(std::stoi(unusedCores));
          aServant.setActiveCores(std::stoi(activeCores));
          aServant.setManagerIpAddress(managerIpAddress);

          if (!db.insertRecord(
            "UPDATE servants SET totalCores = '" + totalCores + "' "
            ", unusedCores = '" + unusedCores + +"' "
            ", activeCores = '" + activeCores + +"' "
            ", managerIpAddress = '" + managerIpAddress + "' "
            ", lastUpdateTime = DATETIME('now') "
            "WHERE ipAddress = '" + aServant.getIpAddress() + "';")) {
            // Log the error and return a 500 response
            comet::Logger::log("Failed to update Settings table with configuration information: ",
                               LoggerLevel::CRITICAL);


            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k500InternalServerError);
            resp->addHeader("Location", "/");
            resp->setBody(R"({"error":"Failed to update Settings table with configuration information"})");
            callback(resp);
            return;
          }

          // Insert into servants if this record does not exist, otherwise just update it
          aServant.updateDatabase(db);

          // Successful update
          auto resp = HttpResponse::newHttpResponse();
          resp->setStatusCode(k302Found);
          resp->addHeader("Location", "/");
          comet::Logger::log("Configuration successfully saved. Redirecting to /", LoggerLevel::INFO);
          callback(resp);
        },
      {Post});


    app().registerHandler(
      "/upload/job/",
      [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          if (!aServant.isManager()) {
            comet::Logger::log(
              "This Servant does not accept jobs since it is not the managing Servant, sumbit to " + aServant.
              getManagerIpAddress(), LoggerLevel::DEBUG);
          }
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
      "/job_summary",
      [this](const HttpRequestPtr &request, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          comet::Logger::log("Handling GET request to /job_summary", LoggerLevel::INFO);

          // Extract sort and filter parameters
          auto sort = request->getParameter("sort");
          auto filter = request->getParameter("filter");

          // Convert sort and filter to lowercase
          std::transform(sort.begin(), sort.end(), sort.begin(), ::tolower);
          std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

          // Validate sort parameter
          if (sort != "status" && sort != "date" && sort != "npv" && sort != "case") {
            sort = "date"; // Default sort
          }

          // Validate filter parameter
          if (filter != "active" && filter != "completed" && filter != "failed" && filter != "all") {
            filter = "all"; // Default filter
          }

          // Generate the job summary report based on sort and filter
          std::string report = Job::jobSummaryHtmlReport(db, sort, filter);

          auto resp = HttpResponse::newHttpResponse();
          resp->setBody(setHTMLBody(report, "/job_summary"));
          callback(resp);
        },
      {Get});

    // Register the /servant/status handler
    drogon::app().registerHandler(
      "/servant/status",
      [this](const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
          // Ensure the request is a POST request
          if (req->method() != drogon::Post) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k405MethodNotAllowed);
            resp->setBody("Method Not Allowed");
            callback(resp);
            return;
          }

          // Parse the JSON body
          auto json = req->getJsonObject();
          if (!json) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k400BadRequest);
            resp->setBody("Invalid JSON properties");
            callback(resp);
            return;
          }

          // Extract and log the posted information
          try {
            aServant.setTotalCores((*json)["totalCores"].asInt());
            aServant.setUnusedCores((*json)["unusedCores"].asInt());
            aServant.setActiveCores((*json)["activeCores"].asInt());
            aServant.setManagerIpAddress((*json)["managerIpAddress"].asString());
            aServant.setIpAddress((*json)["ipAddress"].asString());
            aServant.setVersion((*json)["ServantVersion"].asString());
            aServant.setEmail((*json)["email"].asString());
            aServant.setCode((*json)["code"].asString());
            aServant.setPort((*json)["port"].asInt());
            aServant.setProjectId((*json)["projectId"].asInt());
            aServant.updateServantSettings(db);

            // Respond with success
            auto postData = nlohmann::json{{"status", "success"}, {"message", "Status updated successfully"}};
            auto resp = drogon::HttpResponse::newHttpJsonResponse(postData.dump());
            callback(resp);
          } catch (const std::exception &e) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k400BadRequest);
            resp->setBody("Error processing JSON: " + std::string(e.what()));
            callback(resp);
          }
        },
      {drogon::Post} // Only allow POST requests
    );


    app().registerHandler(
      "/quit",
      [this](const HttpRequestPtr &, std::function<void(const HttpResponsePtr &)> &&callback)
        {
          m_running = false; // Set running to false

          auto resp = HttpResponse::newHttpResponse();
          resp->setBody(setHTMLBody("Server is shutting down...", "/"));
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

std::string WebServices::setHTMLBody(const std::string &body, const std::string &targetPath) const
  {
    return R"(
          <!DOCTYPE html>
          <html>
          )" + getHTMLHeader(targetPath) + R"(
          <body>
          <div style="margin: 1cm;">
          )" + body + R"(
          </div>
          </body>
          )" + getHTMLFooter() + R"(
          </html>
          )";
  }

std::string WebServices::getHTMLHeader(const std::string &targetPath) const
  {
    struct Link
      {
        std::string name;
        std::string href;
      };

    std::vector<Link> links = {
      {"Public", "https://www.cometstrategy.com/"},
      {"Support", "https://support.cometstrategy.com/?site=support&page=dashboard"},
      {"Settings", "/"},
      {"Job Summary", "/job_summary?sort=date&filter=all"},
      {"Servant Status", "/servant/status"},
      {"Quit", "/quit"}
    };

    std::string linksHTML;
    for (const auto &link: links) {
      std::string style = (link.href == targetPath)
                            ? "style='color:#4DB0DD; font-weight: bold; margin-right: 10px'"
                            : "";
      linksHTML += "<a href='" + link.href + "' class='system_link' " + style + " style='margin-right: 10px;'>" + link.
          name + "</a> ";
    }

    return R"(
    <link rel="stylesheet" href="https://support.cometstrategy.com/themes/zCSS.css">
    <link rel='shortcut icon' type='image/png' href='https://media.cometstrategy.com/img/COMET_Icon.png'>
    <header>
        <img src='https://media.cometstrategy.com/img/COMET_DarkBG.svg' alt='1' height='60'>
        <span class='heading_title'>COMET Servants</span>
        <h1>Welcome to COMET Servants</h1>
        )" + linksHTML + R"(
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
        <footer style="margin-top: auto; display: flex; justify-content: space-between; align-items: center;">
            <h4 style="margin: 0;">)" + oss.str() + R"(</h4>
            <h4 style="margin: 0;">&copy; )" + std::to_string(1900 + std::localtime(&now_c)->tm_year) +
           R"( COMET Strategy</h4>
            <h4 style="margin: 0;">Config File: )" + configurationFilePath + R"(</h4>
        </footer>
    )";
  }

void WebServices::run()
  {
    m_serverThread = std::make_unique<std::thread>([this]
      {
        Logger::log(std::string("Server running on localhost:") + to_string(aServant.getPort())
                    + ", Private local IP: " + getPrivateIPAddress() + ":" + to_string(aServant.getPort())
                    + " or Public IP: " + getPublicIPAddressFromWeb() + ":" + to_string(aServant.getPort())
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

bool WebServices::createNewDatabase()
  {
    comet::Logger::log("Database file " + configurationFilePath + " Checking for all tables.",
                       LoggerLevel::INFO);

    aServant.createNewServentsTable(db);


    Job::createNewJobsTable(db);


    comet::Logger::log("Servants and Jobs tables created successfully.", LoggerLevel::INFO);
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
