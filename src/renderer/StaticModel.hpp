#pragma once

#include <renderer/ModelBase.hpp>
#include <renderer/GLUtils.hpp>
#include <utils/Log.hpp>

class StaticModel : public ModelBase {
private:

public:
    bool m_has_vertex_colors;
    bool m_has_texture;
    StaticModel(const char* model_path, Shader* shader = nullptr) {
        if (shader) {
            set_shader(shader);
        }

        _load_model(model_path, 
            aiProcess_JoinIdenticalVertices | 
            aiProcess_Triangulate | 
            aiProcess_GenUVCoords |
            aiProcess_GenNormals
        );

        // Process meshes and materials
        for (unsigned int i = 0; i < num_meshes; ++i) {
            aiMesh* mesh = m_scene->mMeshes[i];
            
            // Check mesh attributes
            m_has_vertex_colors = mesh->HasVertexColors(0);
            m_has_texture = mesh->mMaterialIndex >= 0 && 
                m_scene->mMaterials[mesh->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE) > 0;

            _process_mesh(mesh, i);
            
            // Process material only if the mesh uses textures
            if (m_has_texture) {
                _process_material(m_scene->mMaterials[mesh->mMaterialIndex], i);
            }
        }

        Log::log_success("Static Model " + std::string(model_path) + " loaded successfully with " + 
            std::to_string(num_meshes) + " meshes", __FILE__, __LINE__);

        m_name = std::string(model_path);
        m_importer.FreeScene();
        m_scene = nullptr;
    }

    virtual void draw() override {
        if (!m_shader) {
            Log::log_error_and_terminate("No shader set for model", __FILE__, __LINE__);
        }
        draw(*m_shader, true);
    }

    virtual void draw(Shader& shader) override {
        draw(shader, true);
    }

    virtual void draw(Shader& shader, const bool& use_model_matrix) const override {
        shader.bind();
        
        if (use_model_matrix) {
            shader.set_uniform_mat4f("u_model", get_model_matrix() * m_pre_transform);
        }

        // Set whether we're using vertex colors or texture
        shader.set_uniform_1i("u_has_vertex_colors", m_has_vertex_colors);
        shader.set_uniform_1i("u_has_texture", m_has_texture);
        shader.set_uniform_1i("u_use_repeating_pattern", false);
        
        if (texture_list.size() > 0) {
            shader.set_uniform_1i("u_texture", texture_list.back()->bind(1));
        }

        // Draw each mesh
        for (unsigned int i = 0; i < num_meshes; i++) {
            mesh_list[i]->bind();
            
            if (m_has_texture && mesh_list[i]->texture) {
                shader.set_uniform_1i("u_texture", mesh_list[i]->texture->bind(1));
            }

            GL_Call(glDrawElements(GL_TRIANGLES, mesh_list[i]->get_face_count(), GL_UNSIGNED_INT, 0));
            mesh_list[i]->unbind();
        }
    }

private:
    void _process_mesh(aiMesh* mesh, unsigned int mesh_index) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        _process_vertex_data(mesh, vertices);
        _process_indices(mesh, indices);

        VertexBufferLayout layout = _create_vertex_buffer_layout();

        mesh_list[mesh_index]->init(
            vertices.data(),
            indices.data(),
            vertices.size() * sizeof(float),
            indices.size(),
            layout
        );
    }

    void _process_vertex_data(aiMesh* mesh, std::vector<float>& vertices) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // Position (always present)
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            // Normal (always present due to aiProcess_GenNormals)
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);

            // Vertex Colors
            if (mesh->HasVertexColors(0)) {
                vertices.push_back(mesh->mColors[0][i].r);
                vertices.push_back(mesh->mColors[0][i].g);
                vertices.push_back(mesh->mColors[0][i].b);
                vertices.push_back(mesh->mColors[0][i].a);
            } else {
                // Default color (white)
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);
                vertices.push_back(1.0f);
            }

            // Texture coordinates
            if (mesh->HasTextureCoords(0)) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }
    }

    void _process_indices(aiMesh* mesh, std::vector<unsigned int>& indices) {
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
    }

    VertexBufferLayout _create_vertex_buffer_layout() {
        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        layout.push<float>(4); // vertex color (RGBA)
        layout.push<float>(2); // uv
        return layout;
    }
};