#pragma once

#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>

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

// OpenGL error checking brought to you and parted by:
// https://www.youtube.com/watch?v=FBbPWSOQ0-w&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=10&ab_channel=TheCherno
#define GL_Call(x) s_gl_clear_error();x;s_gl_log_call(#x, __FILE__, __LINE__)

static void s_gl_clear_error() {
    while (glGetError() != GL_NO_ERROR);
}

static void s_gl_log_call(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        Log::log_error_and_terminate(
            std::string(function) + 
            "\n" +
            std::string(file) + 
            ":" + 
            std::to_string(line)
        );
    }
}


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
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_ibo;
    unsigned int m_shader;
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
        GL_Call(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3));
        GL_Call(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3));
        GL_Call(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));
        GL_Call(glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE));
#if __APPLE__
        GL_Call(glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE));
#endif
        GL_Call(glfwWindowHint(GLFW_RESIZABLE, 0));

        m_window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
        if (!m_window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }

        GL_Call(glfwMakeContextCurrent(m_window));
        
        // "Initializes the library. Should be called once after an 
        // OpenGL context has been created. Returns 0 when gl3w was 
        // initialized successfully, non-zero if there was an error."
        // https://github.com/skaslev/gl3w
        if (gl3w_init()) {
            throw std::runtime_error("Failed to initialize gl3w");
        }

        // We are using Opengl 3.3. Basically check A1.
        if (!gl3w_is_supported(3, 3)) {
            throw std::runtime_error("OpenGL 3.1 not supported\n");
        }

        // Binding means that you are selecting. OpenGL is a state machine.
        GL_Call(glGenBuffers(1, &m_vbo));
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));

        // Going to try using index buffer.
        GL_Call(glGenBuffers(1, &m_ibo));
        GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo));
        
        GL_Call(glGenVertexArrays(1, &m_vao));
        GL_Call(glBindVertexArray(m_vao));

        // Set vertex attribute pointers. They basically tell OpenGL the structure of a vertex, so that
        // it can parse them into meaningful data-types once they get to the shaders. The data contained
        // within a Vertex is called an attribute. For example, vertex.position is an attribute.
        // https://docs.gl/gl3/glVertexAttribPointer
        //
        // If no shader is present, then by default: OpenGl will use the attribute @index[0] as the position.
        //
        // If this is confusing, check out this video to learn what an attribute is.: 
        // https://www.youtube.com/watch?v=x0H--CL2tUI&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=5&ab_channel=TheCherno
        GL_Call(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position)));
        GL_Call(glEnableVertexAttribArray(0));
        
        GL_Call(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, colour)));
        GL_Call(glEnableVertexAttribArray(1));

        _load_shaders();
    }

    void draw() {
        // Here are some vertices for testing
        Vertex vertices[] = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0
            {{0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, // 1
            {{0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}, // 2
            {{-0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}, // 3
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 1
        };
        
        GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

        // Try using index buffer..
        GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

        /* Render here */
        GL_Call(glClear(GL_COLOR_BUFFER_BIT));

        // glDrawArrays(GL_TRIANGLES, 0, Common::c_arr_size(vertices));
        GL_Call(glDrawElements(GL_TRIANGLES, Common::c_arr_size(indices), GL_UNSIGNED_INT, &indices));

        /* Swap front and back buffers */
        GL_Call(glfwSwapBuffers(m_window));

        /* Poll for and process events */
        GL_Call(glfwPollEvents());
    }

    bool is_terminated() {
        return bool(glfwWindowShouldClose(m_window));
    }

    void terminate() {
        glfwTerminate();
        glfwSetWindowShouldClose(m_window, 1);
    }

private:
    void _load_shaders() {
        // Gonna use these test shaders for testing.
        std::string hello_vertex_shader;
        std::string hello_fragment_shader;
        
        // this shader makes the triangle red.
        hello_vertex_shader = FileSystem::read_file("shaders/Hello.vs.glsl");
        hello_fragment_shader = FileSystem::read_file("shaders/Hello.fs.glsl");
        
        m_shader = _create_shader(hello_vertex_shader, hello_fragment_shader);
        GL_Call(glUseProgram(m_shader));
    }
    
    // The Cherno goes over initializing shaders in this video. Check it out if you are curious.
    // https://www.youtube.com/watch?v=71BLZwRGUJE&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=7&ab_channel=TheCherno
    unsigned int _compile_shader(const int& type, const std::string& source) {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        GL_Call(glShaderSource(id, 1, &src, nullptr));
        GL_Call(glCompileShader(id));

        int result;
        GL_Call(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

        if (result == GL_FALSE) {
            int length;
            GL_Call(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
            char* error_message = (char*)alloca(length * sizeof(char));
            GL_Call(glGetShaderInfoLog(id, length, &length, error_message));
            std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
            std::cout << error_message << std::endl;
            GL_Call(glDeleteShader(id));
            return 0;
        }

        return id;
    }

    // The Cherno goes over initializing shaders in this video. Check it out if you are curious.
    // https://www.youtube.com/watch?v=71BLZwRGUJE&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=7&ab_channel=TheCherno
    unsigned int _create_shader(const std::string& vertex_shader, const std::string& fragment_shader) {
        unsigned int program = glCreateProgram();
        unsigned int compiled_vertex_shader = _compile_shader(GL_VERTEX_SHADER, vertex_shader);
        unsigned int compiled_fragment_shader = _compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

        GL_Call(glAttachShader(program, compiled_vertex_shader));
        GL_Call(glAttachShader(program, compiled_fragment_shader));
        GL_Call(glLinkProgram(program));
        GL_Call(glValidateProgram(program));

        GL_Call(glDeleteShader(compiled_vertex_shader));
        GL_Call(glDeleteShader(compiled_fragment_shader));
        
        return program;
    }
};