#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <ostream>

namespace Common {
    inline std::vector<std::string> split_string(const std::string& str, const char& delimiter) {
        std::string token;
        std::vector<std::string> tokens;

        std::stringstream stream(str);
        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Was too lazy to make my own. Check this out if you're interested.
    // https://stackoverflow.com/questions/9277906/stdvector-to-string-with-custom-delimiter
    inline std::string join_string(const std::vector<std::string>& tokens, const char& delimiter) {
        std::ostringstream stream;
        for (const auto& token : tokens) {
            if (&token != &tokens[0]) {
                stream << delimiter;
            }
            stream << token;
        }
        return stream.str();
    }

    template<typename T, unsigned int N>
    inline unsigned int c_arr_count(const T (&array)[N]) {
        // Suppress compiler warning...
        (void)array;
        return N;
    }

    inline glm::vec2 normalize(glm::vec2 in) {
        float mag = sqrtf(in.x * in.x + in.y * in.y);
        return {in.x / mag, in.y / mag};
    }
}