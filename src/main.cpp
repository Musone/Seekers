/*
You MUST
        #define GL3W_IMPLEMENTATION
    in EXACLY _one_ C or C++ file that includes this header, BEFORE the include,
    like this:
        #define GL3W_IMPLEMENTATION
            #include "gl3w.h"
    All other files should just #include "gl3w.h" without the #define.

    Found in file "ext/gl3w/gl3w.h"
*/
#define GL3W_IMPLEMENTATION

#include "ecs/Registry.hpp"

// "Note that GL/gl3w.h must be included before any other OpenGL related headers."
// https://github.com/skaslev/gl3w
//
// Adding gl3w breaks the normal OpenGl triangle initialization.
#include <gl3w.h>
#include <GLFW/glfw3.h>
#include <iostream>

int main(void) {
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    
    // "Initializes the library. Should be called once after an 
    // OpenGL context has been created. Returns 0 when gl3w was 
    // initialized successfully, non-zero if there was an error."
    // https://github.com/skaslev/gl3w
    if (gl3w_init()) {
        fprintf(stderr, "failed to initialize OpenGL\n");
        return -1;
    }

    // // We are using Opengl 3.1. Look at the cmake file to see.
    if (!gl3w_is_supported(3, 1)) {
        fprintf(stderr, "OpenGL 3.1 not supported\n");
        return -1;
    }

    // std::cout << glfwGetVersion(GL_VERSION) << std::endl;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
