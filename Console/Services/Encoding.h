//
// Created by Brett King on 7/6/2025.
//

#ifndef BASE64_H
#define BASE64_H

#include <string>
#include <vector>
#include <stdexcept>
#include <codecvt>
#include <locale>

namespace comet 
  {
    class Encoding {
      public:
        // Encode a string to Base64
        static std::string encode(const std::string &input) {
            static const std::string base64_chars =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789+/";

            std::string output;
            int val = 0, valb = -6;
            for (unsigned char c : input) {
              val = (val << 8) + c;
              valb += 8;
              while (valb >= 0) {
                output.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
              }
            }
            if (valb > -6) {
              output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
            }
            while (output.size() % 4) {
              output.push_back('=');
            }
            return output;
        }

        // Decode a Base64 string
        static std::string decode(const std::string &input) {
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
            for (unsigned char c : input) {
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

        // Convert a string to UTF-8
        static inline std::string utf8_encode(const std::string &input) {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.to_bytes(std::wstring(input.begin(), input.end()));
        }

    };
  };
#endif //BASE64_H