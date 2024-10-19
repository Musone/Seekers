#pragma once

#include <renderer/VertexArray.hpp>
#include <renderer/VertexBuffer.hpp>
#include <renderer/VertexBufferLayout.hpp>
#include <renderer/IndexBuffer.hpp>
#include <utils/Common.hpp>

class Mesh {
public:
    VertexArray m_vao;
    VertexBuffer m_vbo;
    IndexBuffer m_ibo;
public:
    Mesh(const void* vertices, const void* indices, const unsigned int& vertices_size, const unsigned int& indices_count, const VertexBufferLayout& layout) {
        m_vao.init();
        m_vbo.init(vertices, vertices_size);
        m_ibo.init(indices, indices_count);
        m_vao.add_buffer(m_vbo, layout);
    };

    ~Mesh() = default;

    unsigned int get_face_count() const { return m_ibo.get_count(); }

    void bind() const {
        m_vao.bind();
        m_vbo.bind();
        m_ibo.bind();
    }

    void unbind() const {
        m_vao.unbind();
        m_vbo.unbind();
        m_ibo.unbind();
    }
private:
    
};