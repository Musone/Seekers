#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <ostream>
#include <glm/glm.hpp>

#include "Transform.hpp"
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

    inline std::string replace_char(const std::string& str, const char& to_be_replaced, const char& replacement) {
        return Common::join_string(Common::split_string(str, to_be_replaced), replacement);
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

    inline float min_of(const float x, const float y) {
        return (x <= y) * x + (y < x) * y;
    }

    inline int min_of(const int x, const int y) {
        return (x <= y) * x + (y < x) * y;
    }

    inline unsigned int min_of(const unsigned int x, const unsigned int y) {
        return (x <= y) * x + (y < x) * y;
    }

    inline float min_of(const glm::vec2 v) {
        return (v.x <= v.y) * v.x + (v.y < v.x) * v.y;
    }

    inline std::string trim(const std::string& str) {
       const char* whitespace = " \t\n\r\f\v";
       size_t start = str.find_first_not_of(whitespace);
       if (start == std::string::npos) return "";

       size_t end = str.find_last_not_of(whitespace);
       return str.substr(start, end - start + 1);
   }


    inline float get_angle_between_item_and_player_view(glm::vec2 item_pos, glm::vec2 player_pos, float player_angle) {
        // Go ask Jakob how this shit works
        auto angle = std::fmod(player_angle, 2 * PI);
        if (angle < 0) {
            angle += 2 * PI;
        }
        auto cos_angle_between_item_and_player = glm::dot(
            glm::normalize(
                glm::vec2(Transform::create_rotation_matrix({0, 0, angle}) * glm::vec4(1, 0, 0, 0))
            ),
            glm::normalize(item_pos - player_pos)
        );
        cos_angle_between_item_and_player = glm::clamp(cos_angle_between_item_and_player, -0.9999999f, 0.9999999f);
        return std::acos(cos_angle_between_item_and_player);
    }
}
