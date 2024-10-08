#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <ostream>

namespace FileSystem{
    inline std::string get_executable_path() {
    #ifdef _WIN32
        char path[MAX_PATH] = { 0 };
        GetModuleFileNameA(NULL, path, MAX_PATH);
    #else
        char path[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
        if (count < 0 || count >= PATH_MAX)
            return "";
        path[count] = '\0';
    #endif
        return std::string(path);
    }

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

    inline std::string read_file(std::string file_path) {
        // Normalize path to use '/' instead of '\\'
        file_path = join_string(split_string(file_path, '\\'), '/');
        // Try absolute path
        std::ifstream file(file_path);
        if (!file.is_open()) {
            // Try relative path
            auto cwd_tokens = split_string(get_executable_path(), '\\');
            // Remove the executable itself from the cwd path
            cwd_tokens.pop_back();
            const auto cwd = join_string(cwd_tokens, '/');
            file.open(cwd + '/' + file_path);
            
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + file_path);
            }
        }
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}