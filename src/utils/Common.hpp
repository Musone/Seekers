#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <ostream>
#include <glm/glm.hpp>

#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "src"; };
inline std::string audio_path(const std::string& name) { return data_path() + "/audio/" + std::string(name); };

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
        if (mag == 0) return { 0, 0 };
        return {in.x / mag, in.y / mag};
    }

    inline float max_of(const float x, const float y) {
        return (x >= y) * x + (y > x) * y;
    }

    inline int max_of(const int x, const int y) {
        return (x >= y) * x + (y > x) * y;
    }

    inline unsigned int max_of(const unsigned int x, const unsigned int y) {
        return (x >= y) * x + (y > x) * y;
    }

    inline float max_of(const glm::vec2 v) {
        return (v.x >= v.y) * v.x + (v.y > v.x) * v.y;
    }
}