#pragma once

#define ANSI_COLOUR_RED "\x1b[31m"
#define ANSI_COLOUR_GREEN "\x1b[32m"
#define ANSI_COLOUR_YELLOW "\x1b[33m"
#define ANSI_COLOUR_RESET "\x1b[0m"

#include <iostream>

namespace Log {
    inline void log_error_and_terminate(const std::string& message, const char* file, int line) {
        std::cerr << ANSI_COLOUR_RED 
            << "\n[ERROR] " 
            << message 
            << '\n' 
            << file
            << ':'
            << std::to_string(line)
            << '\n'
            << ANSI_COLOUR_RESET << std::endl;
        exit(1);  // Terminate the program with error code 1
    }
 
    inline void log_warning(const std::string& message, const char* file, int line) {
        std::cout << ANSI_COLOUR_YELLOW
            << "\n[WARNING] "
            << message
            << '\n'
            << file
            << ':'
            << std::to_string(line)
            << '\n'
            << ANSI_COLOUR_RESET << std::endl;
    }

    inline void log_success(const std::string& message, const char* file, int line) {
        std::cout << ANSI_COLOUR_GREEN
            << "\n[SUCCESS] "
            << message
            << '\n'
            << file
            << ':'
            << std::to_string(line)
            << '\n'
            << ANSI_COLOUR_RESET << std::endl;
    }
}