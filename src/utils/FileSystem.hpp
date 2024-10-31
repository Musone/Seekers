#pragma once

#include <utils/Common.hpp>

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

    inline std::string read_file(std::string file_path) {
        // Normalize path to use '/' instead of '\\'
        file_path = Common::replace_char(file_path, '\\', '/');
        // Try absolute path
        std::ifstream file(file_path);
        if (!file.is_open()) {
            // Try relative path
            auto cwd_tokens = Common::split_string(get_executable_path(), '\\');
            // Remove the executable itself from the cwd path
            cwd_tokens.pop_back();
            const auto cwd = Common::join_string(cwd_tokens, '/');
            file.open(cwd + '/' + file_path);
            
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + file_path);
            }
        }
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}
