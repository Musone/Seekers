#pragma once

#include <renderer/Animator.hpp>
#include <renderer/Animation.hpp>
#include <renderer/GLUtils.hpp>
#include <renderer/Texture2D.hpp>
#include <renderer/Shader.hpp>
#include <renderer/Mesh.hpp>
#include <renderer/Skeleton.hpp>
#include <utils/Transform.hpp>
#include <utils/Log.hpp>

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
public:
    struct Attachment {
        Model* model;
        std::string joint_name;
        // Optional: local offset from joint
        glm::vec3 offset_position = glm::vec3(0.0f);
        glm::vec3 offset_rotation = glm::vec3(0.0f);
        glm::vec3 offset_scale = glm::vec3(1.0f);
    };
private:
    Skeleton m_skeleton;
    Animator m_animator; 
    std::vector<Animation*> m_animations; 

    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;

    Assimp::Importer m_importer;
    const aiScene* m_scene = nullptr;

    std::vector<Attachment> m_attachments;
public:
    std::vector<Mesh> mesh_list;
    std::vector<Texture2D> texture_list;
    unsigned int num_meshes;

    Model(const char* model_path) : 
        m_position(0.0f),
        m_rotation(0.0f),
        m_scale(1.0f),
        m_animator(&m_skeleton) {
        _load_model(model_path);
    }

    ~Model() {
        for (auto anim : m_animations) {
            delete anim;
        }
    }

    void draw(Shader& shader) {
        draw(shader, true);
    }

    void draw(Shader& shader, const bool& use_model_matrix) const {
        shader.bind();
        
        if (use_model_matrix) {
            shader.set_uniform_mat4f("u_model", get_model_matrix());
        }

        // Set animation data if available
        if (m_skeleton.has_joints()) {
            shader.set_uniform_mat4f_array(
                "u_joint_transforms", 
                *m_skeleton.get_joint_transforms().data(), 
                m_skeleton.get_joint_count()
            );
        }

        // Draw each mesh
        for (unsigned int i = 0; i < num_meshes; i++) {
            mesh_list[i].bind();
            
            if (mesh_list[i].m_texture) {
                shader.set_uniform_1i("u_texture", mesh_list[i].m_texture->bind(1));
            }

            GL_Call(glDrawElements(GL_TRIANGLES, mesh_list[i].get_face_count(), GL_UNSIGNED_INT, 0));
            mesh_list[i].unbind();
        }

        // Draw attached models
        for (const auto& attachment : m_attachments) {
            if (attachment.model) {
                // Get joint transform
                const Skeleton::Joint* joint = m_skeleton.get_joint_by_name(attachment.joint_name);
                if (joint) {
                    // Calculate attachment transform
                    glm::mat4 joint_transform = m_skeleton.get_joint_transforms()[joint->id];
                    glm::mat4 attachment_transform = 
                        get_model_matrix() * 
                        joint_transform *
                        Transform::create_model_matrix(
                            attachment.offset_position,
                            attachment.offset_rotation,
                            attachment.offset_scale
                        );

                    // Draw attached model with combined transform
                    shader.set_uniform_mat4f("u_model", attachment_transform);
                    attachment.model->draw(shader, false);
                }
            }
        }
    }

    void update() {
        m_animator.update();
    }

    void play_animation(size_t index) {
        if (index < m_animations.size()) {
            m_animator.set_animation(m_animations[index]);
        }
    }

    size_t get_animation_count() const {
        return m_animations.size();
    }

    unsigned int get_face_count() const {
        Log::log_error_and_terminate("This is deprecated.", __FILE__, __LINE__);
        return 0xdeadbeef;
    }

    void reset_to_bind_pose() {
        if (m_skeleton.has_joints()) {
            m_skeleton.reset_to_bind_pose();
        }
    }

    Attachment* attach_to_joint(
        Model* model, const std::string& joint_name, 
        const glm::vec3& offset_pos = glm::vec3(0.0f),
        const glm::vec3& offset_rot = glm::vec3(0.0f),
        const glm::vec3& offset_scale = glm::vec3(1.0f)
    ) {
        m_attachments.push_back({
            model, 
            joint_name, 
            offset_pos,
            offset_rot,
            offset_scale
        });
        return &m_attachments.back();
    }

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

    glm::mat4 get_model_matrix() const { 
        return Transform::create_model_matrix(m_position, m_rotation, m_scale); 
    }
#pragma endregion

private:
    void _load_model(const char* model_path) {
        m_scene = m_importer.ReadFile(model_path, 
            aiProcess_JoinIdenticalVertices | 
            aiProcess_Triangulate | 
            aiProcess_GenUVCoords |
            aiProcess_LimitBoneWeights |
            aiProcess_GenNormals
        );

        if (!m_scene || !m_scene->mRootNode || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
            Log::log_error_and_terminate("Assimp importer.ReadFile Error: " + 
                std::string(m_importer.GetErrorString()), __FILE__, __LINE__);
        }

        num_meshes = m_scene->mNumMeshes;
        mesh_list.resize(num_meshes);

        // First process the skeleton if any mesh has bones
        for (unsigned int i = 0; i < num_meshes; ++i) {
            aiMesh* mesh = m_scene->mMeshes[i];
            if (mesh->HasBones()) {
                m_skeleton.init_from_mesh(mesh, m_scene);
                break;
            }
        }

        // Then process all meshes
        for (unsigned int i = 0; i < num_meshes; ++i) {
            aiMesh* mesh = m_scene->mMeshes[i];
            _process_mesh(mesh, i);
            _process_material(m_scene->mMaterials[mesh->mMaterialIndex], i);
        }

        Log::log_success("Model loaded successfully with " + std::to_string(num_meshes) + " meshes", 
            __FILE__, __LINE__);

        // Load animations if they exist
        if (m_scene->HasAnimations()) {
            for (unsigned int i = 0; i < m_scene->mNumAnimations; i++) {
                Animation* anim = Animation::from_assimp_animation(m_scene->mAnimations[i], m_scene);
                if (anim) {
                    m_animations.push_back(anim);
                    Log::log_success("Loaded animation " + std::to_string(i), __FILE__, __LINE__);
                }
            }
        }
    }

    void _process_mesh(aiMesh* mesh, unsigned int mesh_index) {
        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        _process_vertex_data(mesh, vertices);

        if (mesh->HasBones()) {
            _process_bone_data(mesh, vertices);
        }

        _process_indices(mesh, indices);

        VertexBufferLayout layout = _create_vertex_buffer_layout();

        mesh_list[mesh_index].init(
            vertices.data(),
            indices.data(),
            vertices.size() * sizeof(float),
            indices.size(),
            layout
        );
    }

    void _process_vertex_data(aiMesh* mesh, std::vector<float>& vertices) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // Position
            vertices.push_back(mesh->mVertices[i].x);
            vertices.push_back(mesh->mVertices[i].y);
            vertices.push_back(mesh->mVertices[i].z);

            // Normal
            if (mesh->HasNormals()) {
                vertices.push_back(mesh->mNormals[i].x);
                vertices.push_back(mesh->mNormals[i].y);
                vertices.push_back(mesh->mNormals[i].z);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }

            // Texture coordinates
            if (mesh->HasTextureCoords(0)) {
                vertices.push_back(mesh->mTextureCoords[0][i].x);
                vertices.push_back(mesh->mTextureCoords[0][i].y);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }

            // Initialize joint IDs with -1 (indicating unused joints)
            for (int j = 0; j < 4; j++) {
                vertices.push_back(-1);
            }

            // Initialize weights with 0
            for (int j = 0; j < 4; j++) {
                vertices.push_back(0.0f);
            }
        }
    }

    void _process_bone_data(aiMesh* mesh, std::vector<float>& vertices) {
        if (!mesh->HasBones()) return;

        std::vector<std::vector<std::pair<int, float>>> vertex_weights(mesh->mNumVertices);

        for (unsigned int bone_index = 0; bone_index < mesh->mNumBones; bone_index++) {
            aiBone* bone = mesh->mBones[bone_index];
            
            for (unsigned int weight_index = 0; weight_index < bone->mNumWeights; weight_index++) {
                aiVertexWeight weight = bone->mWeights[weight_index];
                vertex_weights[weight.mVertexId].push_back({bone_index, weight.mWeight});
            }
        }

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            auto& weights = vertex_weights[i];
            
            std::sort(weights.begin(), weights.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });

            size_t base_offset = i * (3 + 3 + 2 + 4 + 4);
            size_t joint_offset = base_offset + 8;
            size_t weight_offset = joint_offset + 4;

            float total_weight = 0.0f;

            for (size_t j = 0; j < std::min(size_t(4), weights.size()); j++) {
                vertices[joint_offset + j] = reinterpret_cast<float&>(weights[j].first);
                vertices[weight_offset + j] = weights[j].second;
                total_weight += weights[j].second;
            }

            if (total_weight > 0.0f) {
                for (int j = 0; j < 4; j++) {
                    vertices[weight_offset + j] /= total_weight;
                }
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

    void _process_material(aiMaterial* material, unsigned int mesh_index) {
        for (unsigned int tex_count = 0; tex_count < material->GetTextureCount(aiTextureType_DIFFUSE); ++tex_count) {
            aiString texture_path;
            material->GetTexture(aiTextureType_DIFFUSE, tex_count, &texture_path);
            std::string texture_name = Common::split_string(texture_path.C_Str(), '/').back();
            
            Texture2D* texture = _get_texture(texture_name.c_str());
            if (texture == nullptr) {
                texture_list.emplace_back(texture_name.c_str());
                texture = &texture_list.back();
                Log::log_success("Loaded texture: " + texture_name, __FILE__, __LINE__);
            }
            mesh_list[mesh_index].set_texture(texture);
        }
    }

    VertexBufferLayout _create_vertex_buffer_layout() {
        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        layout.push<float>(2); // uv
        layout.push<int>(4);   // joint indices (vec4)
        layout.push<float>(4); // weights (vec4)
        return layout;
    }

    Texture2D* _get_texture(const std::string& texture_name) {
        for (unsigned int i = 0; i < texture_list.size(); ++i) {
            if (texture_name == texture_list[i].get_file_name()) {
                return &texture_list[i];
            }
        }
        return nullptr;
    }
};