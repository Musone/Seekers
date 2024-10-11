#pragma once

#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>
#include <renderer/VertexBuffer.hpp>
#include <renderer/IndexBuffer.hpp>
#include <renderer/VertexArray.hpp>
#include <renderer/VertexBufferLayout.hpp>
#include <renderer/GLUtils.hpp>
#include <renderer/Shader.hpp>

#include <string>

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

// "Note that GL/gl3w.h must be included before any other OpenGL related headers."
// https://github.com/skaslev/gl3w
//
// Adding gl3w breaks the normal OpenGl triangle initialization.
#include <gl3w.h>
#include <GLFW/glfw3.h>

// We are using Opengl 3.3
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 3

// Here, I am defining what a Vertex is. This struct is used to tell OpenGL what Attributes
// it should expect when reading in a Vertex. Using this method helps prevent hardcoding magic numbers.
struct Vertex {
    glm::vec2 position = { 0.0f, 0.0f };
    glm::vec3 colour = { 0.0f, 0.0f, 0.0f };
};

// Eventually going to wrap OpenGL with this class.
class Renderer {
private:
    GLFWwindow* m_window;
    VertexArray m_vao;
    VertexBuffer m_vbo;
    IndexBuffer m_ibo;
    Shader m_shader;
public:
    // Make sure to catch, log, and terminate errors when using the renderer.
    void init() {
        // Magic code that sets up OpenGL. The order of these calls matter. Best not
        // to touch it ;)
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        //-------------------------------------------------------------------------
        // If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
        // enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
        // GLFW / OGL Initialization
        // TODO: FIX THIS RANDOM ERROR WITH GL_CALL......
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
        glfwWindowHint(GLFW_RESIZABLE, 0);

        m_window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
        if (!m_window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }

        glfwMakeContextCurrent(m_window);
        
        // "Initializes the library. Should be called once after an 
        // OpenGL context has been created. Returns 0 when gl3w was 
        // initialized successfully, non-zero if there was an error."
        // https://github.com/skaslev/gl3w
        if (gl3w_init()) {
            throw std::runtime_error("Failed to initialize gl3w");
        }

        if (!gl3w_is_supported(GL_VERSION_MAJOR, GL_VERSION_MINOR)) {
            throw std::runtime_error("OpenGL " + std::to_string(GL_VERSION_MAJOR) + "." + std::to_string(GL_VERSION_MINOR) + " not supported\n");
        }
    
        _test_demo_setup();
    }

    void draw() {
        /* Render here */
        GL_Call(glClear(GL_COLOR_BUFFER_BIT));

        // GL_Call(glUseProgram(m_shader));

        m_shader.bind();
        m_shader.set_uniform_4f("u_coooooolor", { 0, 0, 1, 1 });

        // When we create the VBO layout (attrib pointer), they get linked internally by OpenGL, and
        // and are stored in the VAO. We only need to bind the VAO after linking everything properly.
        m_vao.bind();

        // We still need the IBO group Vertices to draw triangles.
        m_ibo.bind();

        // GL_Call(glDrawElements(GL_TRIANGLES, Common::c_arr_count(indices), GL_UNSIGNED_INT, &indices));
        GL_Call(glDrawElements(GL_TRIANGLES, m_ibo.get_count(), GL_UNSIGNED_INT, 0));
        
        /* Swap front and back buffers */
        GL_Call(glfwSwapBuffers(m_window));

        /* Poll for and process events */
        GL_Call(glfwPollEvents());
    }

    bool is_terminated() {
        return bool(glfwWindowShouldClose(m_window));
    }

    void terminate() {
        GL_Call(glfwTerminate());
        GL_Call(glfwSetWindowShouldClose(m_window, 1));
    }

private:
    void _test_demo_setup() {
        // Here are some vertices for testing
        Vertex vertices[] = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0
            {{0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, // 1
            {{0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}, // 2
            {{-0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}, // 3
        };

        m_vao.init(); // this should already be initialized because of the constructor...
        m_vbo.init(vertices, sizeof(vertices));
        
        VertexBufferLayout layout;
        layout.push<float>(2);
        layout.push<float>(3);

        // Link this Attribute layour to the VBO
        m_vao.add_buffer(m_vbo, layout);

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 1
        };

        // Index buffer lets us reuse vertices to reduce vram consumption.
        m_ibo.init(indices, Common::c_arr_count(indices));

        // this shader makes the triangle red. Shader is expected to be in src/shaders/
        m_shader.init("Hello");
    }
};