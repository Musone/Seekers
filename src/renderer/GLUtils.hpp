#pragma once

#include <utils/Log.hpp>

#include <string>

#include <gl3w.h>
// #include <GLFW/glfw3.h>

// OpenGL error checking brought to you and parted by:
// https://www.youtube.com/watch?v=FBbPWSOQ0-w&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=10&ab_channel=TheCherno
#define GL_Call(x) s_gl_clear_error();x;s_gl_log_call(#x, __FILE__, __LINE__)

static void s_gl_clear_error() {
    while (glGetError() != GL_NO_ERROR);
}

static void s_gl_log_call(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        Log::log_error_and_terminate(std::string(function), file, line);
    }
}