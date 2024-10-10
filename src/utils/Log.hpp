#pragma once

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <iostream>

namespace Log {
    inline void log_error_and_terminate(const std::string& message, const char* file, int line) {
        std::cerr << ANSI_COLOR_RED 
            << "\nError: " 
            << message 
            << '\n' 
            << file
            << ':'
            << std::to_string(line)
            << '\n'
            << ANSI_COLOR_RESET << std::endl;
        exit(1);  // Terminate the program with error code 1
    }
}