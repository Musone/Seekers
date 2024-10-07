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

#pragma region Setup_modern_gl_lol
    // Define vertices for a triangle
    float vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    // Create and bind Vertex Array Object (VAO)
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Create and bind Vertex Buffer Object (VBO)
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
#pragma endregion

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Use the shader program
        // Not using shaders yet.
        // glUseProgram(shaderProgram);

        // Draw the triangle
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
