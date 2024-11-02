#pragma once

#include <renderer/Shader.hpp>
#include <renderer/Mesh.hpp>
#include <renderer/GLUtils.hpp>
#include <renderer/Texture2D.hpp>
#include <utils/Transform.hpp>
#include <utils/Log.hpp>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class ModelBase {
protected:
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;

    glm::mat4 m_pre_transform;

    std::string m_name = "No Name";

    Assimp::Importer m_importer;
    const aiScene* m_scene = nullptr;
    Shader* m_shader;

public:
    std::vector<std::shared_ptr<Mesh>> mesh_list;
    std::vector<std::shared_ptr<Texture2D>> texture_list;
    unsigned int num_meshes;

    ModelBase() : 
        m_position(0.0f),
        m_rotation(0.0f),
        m_scale(1.0f),
        num_meshes(0),
        m_shader(nullptr),
        m_pre_transform(1.0f)
    {}

    virtual ~ModelBase() = default;

    // Shader management
    void set_shader(Shader* shader) { m_shader = shader; }
    Shader* get_shader() const { return m_shader; }

    // Draw methods
    virtual void draw() {
        if (!m_shader) {
            Log::log_error_and_terminate("No shader set for model", __FILE__, __LINE__);
        }
        draw(*m_shader, true);
    }

    virtual void draw(Shader& shader) {
        draw(shader, true);
    }

    virtual void draw(Shader& shader, const bool& use_model_matrix) const = 0;

#pragma region Transform Operations
    // Basic vector setters
    void set_position(const glm::vec3& position) { m_position = position; }
    void set_rotation(const glm::vec3& rotation) { m_rotation = rotation; }
    void set_scale(const glm::vec3& scale) { m_scale = scale; }

    // Component-wise setters
    void set_position(float x, float y, float z) { m_position = glm::vec3(x, y, z); }
    void set_rotation(float x, float y, float z) { m_rotation = glm::vec3(x, y, z); }
    void set_scale(float x, float y, float z) { m_scale = glm::vec3(x, y, z); }

    // Individual component setters
    void set_position_x(float x) { m_position.x = x; }
    void set_position_y(float y) { m_position.y = y; }
    void set_position_z(float z) { m_position.z = z; }

    void set_rotation_x(float x) { m_rotation.x = x; }
    void set_rotation_y(float y) { m_rotation.y = y; }
    void set_rotation_z(float z) { m_rotation.z = z; }

    void set_scale_x(float x) { m_scale.x = x; }
    void set_scale_y(float y) { m_scale.y = y; }
    void set_scale_z(float z) { m_scale.z = z; }

    // Basic vector getters
    const glm::vec3& get_position() const { return m_position; }
    const glm::vec3& get_rotation() const { return m_rotation; }
    const glm::vec3& get_scale() const { return m_scale; }

    // Individual component getters
    float get_position_x() const { return m_position.x; }
    float get_position_y() const { return m_position.y; }
    float get_position_z() const { return m_position.z; }

    float get_rotation_x() const { return m_rotation.x; }
    float get_rotation_y() const { return m_rotation.y; }
    float get_rotation_z() const { return m_rotation.z; }

    float get_scale_x() const { return m_scale.x; }
    float get_scale_y() const { return m_scale.y; }
    float get_scale_z() const { return m_scale.z; }

    // Relative movement methods
    void translate(const glm::vec3& delta) { m_position += delta; }
    void rotate(const glm::vec3& delta) { m_rotation += delta; }
    void scale(const glm::vec3& delta) { m_scale *= delta; }

    void translate(float x, float y, float z) { m_position += glm::vec3(x, y, z); }
    void rotate(float x, float y, float z) { m_rotation += glm::vec3(x, y, z); }
    void scale(float x, float y, float z) { m_scale *= glm::vec3(x, y, z); }

    void set_pre_transform(const glm::mat4& pre_transform) {
        m_pre_transform = pre_transform;
    }

    glm::mat4 get_model_matrix() const { 
        return Transform::create_model_matrix(m_position, m_rotation, m_scale); 
    }

    std::string get_name() const { return m_name; };
#pragma endregion

protected:
    virtual void _load_model(const char* model_path, unsigned int import_flags) {
        m_scene = m_importer.ReadFile(model_path, import_flags);

        if (!m_scene || !m_scene->mRootNode || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            Log::log_error_and_terminate("Assimp importer.ReadFile Error: " + 
                std::string(m_importer.GetErrorString()), __FILE__, __LINE__);
        }

        num_meshes = m_scene->mNumMeshes;
        mesh_list.reserve(num_meshes);
        for (size_t i = 0; i < num_meshes; i++) {
            mesh_list.push_back(std::make_shared<Mesh>());
        }
    }

    virtual void _process_material(aiMaterial* material, unsigned int mesh_index) {
        for (unsigned int tex_count = 0; tex_count < material->GetTextureCount(aiTextureType_DIFFUSE); ++tex_count) {
            aiString texture_path;
            material->GetTexture(aiTextureType_DIFFUSE, tex_count, &texture_path);
            std::string texture_name = Common::split_string(texture_path.C_Str(), '/').back();
            
            std::shared_ptr<Texture2D> texture = _get_texture(texture_name.c_str());
            if (texture == nullptr) {
                texture_list.push_back(std::make_shared<Texture2D>(texture_name.c_str()));
                texture = texture_list.back();
                Log::log_success("Loaded texture: " + texture_name, __FILE__, __LINE__);
            }
            mesh_list[mesh_index]->set_texture(texture);
        }
    }

    std::shared_ptr<Texture2D> _get_texture(const std::string& texture_name) {
        for (auto& texture : texture_list) {
            if (texture_name == texture->get_file_name()) {
                return texture;
            }
        }
        return nullptr;
    }
};