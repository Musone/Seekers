#pragma once

#include <renderer/VertexArray.hpp>
#include <renderer/VertexBuffer.hpp>
#include <renderer/VertexBufferLayout.hpp>
#include <renderer/IndexBuffer.hpp>
#include <renderer/Texture2D.hpp>
#include <utils/Common.hpp>
#include <utils/Log.hpp>
#include <utils/Triangle.hpp>

class Mesh {
private:
    bool m_is_initialized = false;
    
    VertexArray m_vao;
    VertexBuffer m_vbo;
    IndexBuffer m_ibo;
public:
    // std::vector<glm::vec3> positions;
    // std::vector<glm::vec3> normals;
    // std::vector<glm::vec2> texcoords;
    std::vector<Triangle> triangles;
    std::shared_ptr<Texture2D> texture = nullptr;
    

    Mesh() = default;

    Mesh(const void* vertices, const void* indices, const unsigned int& vertices_size, const unsigned int& indices_count, const VertexBufferLayout& layout) {
        init(vertices, indices, vertices_size, indices_count, layout);
    };

    ~Mesh() = default;

    void init(const void* vertices, const void* indices, const unsigned int& vertices_size, const unsigned int& indices_count, const VertexBufferLayout& layout) {

#pragma region Keep track of triangles for mesh collision
        unsigned int n_triangles = indices_count / 3;
        unsigned int stride_in_floats = layout.get_stride() / sizeof(float);
        const float* f_vertices = (const float*)vertices;
        const unsigned int* ui_indices = (const unsigned int*)indices;

        triangles.resize(n_triangles);
        for (unsigned int i = 0; i < n_triangles; ++i) {
            Triangle* t = &triangles[i];
            unsigned int ui_indices_base_index = i * 3;
            t->v0 = {
                f_vertices[ui_indices[ui_indices_base_index + 0] * stride_in_floats + 0],
                f_vertices[ui_indices[ui_indices_base_index + 0] * stride_in_floats + 1],
                f_vertices[ui_indices[ui_indices_base_index + 0] * stride_in_floats + 2],
            };
            t->v1 = {
                f_vertices[ui_indices[ui_indices_base_index + 1] * stride_in_floats + 0],
                f_vertices[ui_indices[ui_indices_base_index + 1] * stride_in_floats + 1],
                f_vertices[ui_indices[ui_indices_base_index + 1] * stride_in_floats + 2],
            };
            t->v2 = {
                f_vertices[ui_indices[ui_indices_base_index + 2] * stride_in_floats + 0],
                f_vertices[ui_indices[ui_indices_base_index + 2] * stride_in_floats + 1],
                f_vertices[ui_indices[ui_indices_base_index + 2] * stride_in_floats + 2],
            };
        }
#pragma endregion

        m_vao.init();
        m_vbo.init(vertices, vertices_size);
        m_ibo.init(indices, indices_count);
        m_vao.add_buffer(m_vbo, layout);
        m_is_initialized = true;
    }

    unsigned int get_face_count() const { return m_ibo.get_count(); }

    const void set_texture(std::shared_ptr<Texture2D> texture) {
        if (!m_is_initialized) Log::log_error_and_terminate("Mesh not initialized", __FILE__, __LINE__);
        this->texture = texture;
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