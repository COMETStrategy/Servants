//
// Created by Joshua King on 19/12/17.
//

#include "Curl.hpp"
#include <curl/curl.h>
#include <memory>

namespace comet
    {
        Curl::Response Curl::get(const std::string &url, std::list<std::string> headers) {
                struct CurlDeleter {
                        void operator()(CURL *ptr) const { curl_easy_cleanup(ptr); }
                };
                std::unique_ptr<CURL, CurlDeleter> curl(curl_easy_init());
                if (!curl) {
                    return Response(ResponseType::CurlError, "Could not instantiate CURL");
                }

                struct CurlSlistDeleter {
                        void operator()(curl_slist *ptr) const { curl_slist_free_all(ptr); }
                };
                std::unique_ptr<curl_slist, CurlSlistDeleter> curlHeaders(nullptr);
                for (const auto &header : headers) {
                    curlHeaders.reset(curl_slist_append(curlHeaders.release(), header.c_str()));
                }

                std::string curlBody;
                long httpCode = 0;

                curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, curlHeaders.get());
                curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, curlWriteCallback);
                curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curlBody);

                auto result = curl_easy_perform(curl.get());

                if (result != CURLE_OK) {
                    return Response(ResponseType::CurlError, std::string(curl_easy_strerror(result)));
                }

                curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &httpCode);
                if (httpCode != 200) {
                    return Response(ResponseType::Error, httpCode, curlBody);
                }

                return Response(ResponseType::Ok, httpCode, curlBody);
        }

        Curl::Response Curl::post(const std::string &url, const std::string &data, std::list<std::string> headers) {
                struct CurlDeleter {
                        void operator()(CURL *ptr) const { curl_easy_cleanup(ptr); }
                };
                std::unique_ptr<CURL, CurlDeleter> curl(curl_easy_init());
                if (!curl) {
                    return Response(ResponseType::CurlError, "Could not instantiate CURL");
                }

                struct CurlSlistDeleter {
                        void operator()(curl_slist *ptr) const { curl_slist_free_all(ptr); }
                };
    
                std::unique_ptr<curl_slist, CurlSlistDeleter> curlHeaders(nullptr);
                for (const auto &header : headers) {
                    curlHeaders.reset(curl_slist_append(curlHeaders.release(), header.c_str()));
                }

                std::string curlBody;
                long httpCode = 0;

                curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, data.c_str());
                curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, curlHeaders.get());
                curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
                curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, curlWriteCallback);
                curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &curlBody);

                auto result = curl_easy_perform(curl.get());

                if (result != CURLE_OK) {
                    return Response(ResponseType::CurlError, std::string(curl_easy_strerror(result)));
                }

                curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &httpCode);
                if (httpCode != 200) {
                    return Response(ResponseType::Error, httpCode, curlBody);
                }

                return Response(ResponseType::Ok, httpCode, curlBody);
        }

        Curl::Response Curl::postJson(const std::string &url, const nlohmann::json &data, std::list<std::string> headers) {
                headers.emplace_back("Content-Type: application/json");
                return post(url, data.dump(), headers);
        }

        size_t Curl::curlWriteCallback(char *contents, size_t size, size_t nmemb, void *userp) {
                ((std::string *) userp)->append(contents, size * nmemb);
                return size * nmemb;
        }
    }