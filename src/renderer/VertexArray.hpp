#pragma once

#include <renderer/GLUtils.hpp>
#include <renderer/VertexBuffer.hpp>
#include <renderer/VertexBufferLayout.hpp>

#include <gl3w.h>
#include <GLFW/glfw3.h>

class VertexArray {
    unsigned int m_id;
    bool m_is_initialized;
public: 
    VertexArray(): m_is_initialized(false) {}

    void init() {
        GL_Call(glGenVertexArrays(1, &m_id));
        m_is_initialized = true;
    }

    ~VertexArray() {
        if (m_is_initialized) {
            GL_Call(glDeleteVertexArrays(1, &m_id));
        }
    }

    void bind() {
        if (!m_is_initialized) Log::log_error_and_terminate("Vertex Array not initialized", __FILE__, __LINE__);
        GL_Call(glBindVertexArray(m_id));
    }

    void unbind() {
        if (!m_is_initialized) Log::log_error_and_terminate("Vertex Array not initialized", __FILE__, __LINE__);
        GL_Call(glBindVertexArray(0));
    }

    void add_buffer (const VertexBuffer& vertex_buffer, const VertexBufferLayout& layout) {
        bind();
        vertex_buffer.bind();
        const auto& elements = layout.get_elements();
        unsigned int offset = 0;

        for (unsigned int i = 0; i < elements.size(); ++i) {
            const auto& element = elements[i];
            GL_Call(glEnableVertexAttribArray(i));
            GL_Call(glVertexAttribPointer(
                i,
                element.count,
                element.type,
                element.is_normalized,
                layout.get_stride(),
                (const void*)offset
            ));

            offset += element.count * LayoutElement::get_size_of_type(element.type);
        }
        
    }
};