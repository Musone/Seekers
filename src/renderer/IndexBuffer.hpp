#pragma once

#include <renderer/GLUtils.hpp>

// "Note that GL/gl3w.h must be included before any other OpenGL related headers."
// https://github.com/skaslev/gl3w
//
// Adding gl3w breaks the normal OpenGl triangle initialization.
#include <gl3w.h>
#include <GLFW/glfw3.h>

// Cherno's abstraction for buffers.
// https://www.youtube.com/watch?v=bTHqmzjm2UI&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=13&ab_channel=TheCherno
class IndexBuffer {
    unsigned int m_id;
    unsigned int m_count;
    bool m_is_initialized;
public:
    IndexBuffer(): m_is_initialized(false) {}

    // 'count' is number of elements.
    IndexBuffer(const void* data, unsigned int count) {
        init(data, count);
    }
        
    void init(const void* data, unsigned int count) {
        m_count = count;
        // Binding means that you are selecting. OpenGL is a state machine.
        GL_Call(glGenBuffers(1, &m_id));
        GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));

        if (sizeof(unsigned int) != sizeof(GLuint)) {
            Log::log_error_and_terminate("Error GLuint is not the same as unsigned int. Consider using GLuint instead\n", __FILE__,  __LINE__);
        }

        GL_Call(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
        m_is_initialized = true;
    }

    ~IndexBuffer() {
        if (m_is_initialized) {
            GL_Call(glDeleteBuffers(1, &m_id));
        }
    }

    void bind() const {
        if (!m_is_initialized) Log::log_error_and_terminate("Index Buffer not initialized", __FILE__, __LINE__);
        GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id));
    }

    void unbind() const {
        if (!m_is_initialized) Log::log_error_and_terminate("Index Buffer not initialized", __FILE__, __LINE__);
        GL_Call(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    }

    inline unsigned int get_count() const { return m_count; };
};