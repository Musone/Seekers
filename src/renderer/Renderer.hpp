#pragma once

#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>
#include <renderer/VertexBuffer.hpp>
#include <renderer/IndexBuffer.hpp>
#include <renderer/VertexArray.hpp>
#include <renderer/GLUtils.hpp>
#include <renderer/Shader.hpp>
#include <renderer/Texture.hpp>

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
    GLFWwindow* m_window;
    bool m_is_initialized;
    int m_window_width;
    int m_window_height;

    Renderer() : m_is_initialized(false) {}
public:
    Renderer(Renderer const&) = delete;
	void operator=(Renderer const&) = delete;

	static Renderer& get_instance() {
		static Renderer instance;
		return instance;
	}

    // Make sure to catch, log, and terminate errors when using the renderer.
    void init(std::string window_name, int window_width, int window_height, bool enable_vsync, bool enable_resize) {
        m_window_width = window_width;
        m_window_height = window_height;
        
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
        glfwWindowHint(GLFW_RESIZABLE, (unsigned int)enable_resize);

        // vsync
        glfwSwapInterval((unsigned int)enable_vsync);

        m_window = glfwCreateWindow(window_width, window_height, window_name.c_str(), NULL, NULL);
        if (!m_window) {
            glfwTerminate();
            Log::log_error_and_terminate("Failed to create window", __FILE__, __LINE__);
        }

        glfwMakeContextCurrent(m_window);
        
        // "Initializes the library. Should be called once after an 
        // OpenGL context has been created. Returns 0 when gl3w was 
        // initialized successfully, non-zero if there was an error."
        // https://github.com/skaslev/gl3w
        if (gl3w_init()) {
            Log::log_error_and_terminate("Failed to initialize gl3w", __FILE__, __LINE__);
        }

        if (!gl3w_is_supported(GL_VERSION_MAJOR, GL_VERSION_MINOR)) {
            Log::log_error_and_terminate(
                "OpenGL " + std::to_string(GL_VERSION_MAJOR) + "." + std::to_string(GL_VERSION_MINOR) + " not supported\n",
                __FILE__, 
                __LINE__
            );
        }
    
        // This is for textures so that transparency blends properly.
        GL_Call(glEnable(GL_BLEND));
        GL_Call(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GL_Call(glEnable(GL_DEPTH_TEST));

        m_is_initialized = true;
        Log::log_success("Renderer loaded", __FILE__, __LINE__);
    }

    const void begin_draw() const {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Renderer not initialized", __FILE__, __LINE__);
        }
        GL_Call(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    const void end_draw() const {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Renderer not initialized", __FILE__, __LINE__);
        }
        /* Swap front and back buffers */
        GL_Call(glfwSwapBuffers(m_window));
        /* Poll for and process events */
        GL_Call(glfwPollEvents());
    }

    const int get_window_width() const { return m_window_width; }
    const int get_window_height() const { return m_window_height; }

    const void draw(const VertexArray& vao, const IndexBuffer& ibo, const Shader& shader) const {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Renderer not initialized", __FILE__, __LINE__);
        }
        shader.bind();
        // When we create the VBO layout (attrib pointer), they get linked internally by OpenGL, and
        // and are stored in the VAO. We only need to bind the VAO after linking everything properly.
        vao.bind();
        // The IBO tells us which triplets of vertices to use for each triangle.
        ibo.bind();
        GL_Call(glDrawElements(GL_TRIANGLES, ibo.get_count(), GL_UNSIGNED_INT, nullptr));
    }

    bool is_terminated() {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Renderer not initialized", __FILE__, __LINE__);
        }
        return bool(glfwWindowShouldClose(m_window));
    }

    void terminate() {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Renderer not initialized", __FILE__, __LINE__);
        }
        GL_Call(glfwTerminate());
        GL_Call(glfwSetWindowShouldClose(m_window, 1));
    }

    int is_key_pressed(const int& key_code) const {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Renderer not initialized", __FILE__, __LINE__);
        }
        GL_Call(int status = glfwGetKey(m_window, key_code));
        return status == GLFW_PRESS;
    }

    // https://www.glfw.org/docs/latest/input_guide.html
    void set_on_key_callback_fn(const void* callback) const {
        GL_Call(glfwSetKeyCallback(m_window, (GLFWkeyfun)callback));
    }

    // https://www.glfw.org/docs/latest/input_guide.html
    void set_on_mouse_move_callback_fn(const void* callback) const {
        GL_Call(glfwSetCursorPosCallback(m_window, (GLFWcursorposfun)callback));
    }

    void set_on_mouse_click_callback_fn(const void* callback) const {
        GL_Call(glfwSetMouseButtonCallback(m_window, (GLFWmousebuttonfun)callback));
    }
};