//
// Created by Joshua King on 19/12/17.
//

#pragma once

#include <string>
#include <utility>
#include <nlohmann/json.hpp>
#include <list>

namespace comet
  {
    class Curl {
      private:
        static size_t curlWriteCallback(char *contents, size_t size, size_t nmemb, void *userp);
      public:
        enum class ResponseType { Ok, Error, CurlError };

        class Response {
          public:
            ResponseType type;
            std::string body;
            std::string curlError;
            long status;

            Response(ResponseType type, long status, std::string body) : type(type), status(status), body(std::move(body)), curlError("") {
            };

            Response(ResponseType type, std::string curlError) : Response(type, 0, "") {
                this->curlError = std::move(curlError);
            };

            bool isError() {
                return type != ResponseType::Ok;
            }

            bool isEmpty() {
                return status == 0 || body.empty();
            }
        };

        static Response postJson(const std::string &url, const nlohmann::json &data, std::list<std::string> headers = {});
        static Response get(const std::string& url, std::list<std::string> headers);
        static Response post(const std::string &url, const std::string &data, std::list<std::string> headers = {});
    };
  }