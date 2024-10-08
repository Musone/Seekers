#pragma once

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#include <iostream>

namespace Log {
    inline void log_error_and_terminate(const std::string& message) {
        std::cerr << ANSI_COLOR_RED << "\n\nError: " << message << "\n\n" << ANSI_COLOR_RESET << std::endl;
        exit(1);  // Terminate the program with error code 1
    }
}