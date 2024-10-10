#pragma once

#include <renderer/GLUtils.hpp>
#include <utils/Log.hpp>

#include <gl3w.h>
#include <GLFW/glfw3.h>

// Cherno's abstraction for buffers.
// https://www.youtube.com/watch?v=bTHqmzjm2UI&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=13&ab_channel=TheCherno
class VertexBuffer {
    unsigned int m_id;
    bool m_is_initialized;
public:
    // 'size' is number of bytes.
    VertexBuffer(): m_is_initialized(false) {}

    VertexBuffer(const void* data, unsigned int size) {
        init(data, size);
    }

    void init(const void* data, unsigned int size) {
        // Binding means that you are selecting. OpenGL is a state machine.
        GL_Call(glGenBuffers(1, &m_id));
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, m_id));
        GL_Call(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
        m_is_initialized = true;
    }

    ~VertexBuffer() {
        if (m_is_initialized) {
            GL_Call(glDeleteBuffers(1, &m_id));
        }
    }

    void bind() const {
        if (!m_is_initialized) Log::log_error_and_terminate("Vertex Buffer not initialized", __FILE__, __LINE__);
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, m_id));
    }

    void unbind() const {
        if (!m_is_initialized) Log::log_error_and_terminate("Vertex Buffer not initialized", __FILE__, __LINE__);
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
};