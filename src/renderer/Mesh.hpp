#pragma once

#include <renderer/VertexArray.hpp>
#include <renderer/VertexBuffer.hpp>
#include <renderer/VertexBufferLayout.hpp>
#include <renderer/IndexBuffer.hpp>
#include <renderer/Texture2D.hpp>
#include <utils/Common.hpp>
#include <utils/Log.hpp>

class Mesh {
public:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    VertexArray m_vao;
    VertexBuffer m_vbo;
    IndexBuffer m_ibo;
    Texture2D* m_texture;
    bool m_is_initialized = false;
public:
    Mesh() = default;

    Mesh(const void* vertices, const void* indices, const unsigned int& vertices_size, const unsigned int& indices_count, const VertexBufferLayout& layout) {
        init(vertices, indices, vertices_size, indices_count, layout);
    };

    ~Mesh() = default;

    void init(const void* vertices, const void* indices, const unsigned int& vertices_size, const unsigned int& indices_count, const VertexBufferLayout& layout) {
        m_vao.init();
        m_vbo.init(vertices, vertices_size);
        m_ibo.init(indices, indices_count);
        m_vao.add_buffer(m_vbo, layout);
        m_is_initialized = true;
    }

    unsigned int get_face_count() const { return m_ibo.get_count(); }

    const void set_texture(Texture2D* texture) {
        if (!m_is_initialized) Log::log_error_and_terminate("Mesh not initialized", __FILE__, __LINE__);
        m_texture = texture;
    }

    void bind() const {
        if (!m_is_initialized) Log::log_error_and_terminate("Mesh not initialized", __FILE__, __LINE__);
        m_vao.bind();
        m_vbo.bind();
        m_ibo.bind();
    }

    void unbind() const {
        if (!m_is_initialized) Log::log_error_and_terminate("Mesh not initialized", __FILE__, __LINE__);
        m_vao.unbind();
        m_vbo.unbind();
        m_ibo.unbind();
    }
private:
    
};