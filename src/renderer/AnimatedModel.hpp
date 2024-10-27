#pragma once

#include <renderer/ModelBase.hpp>
#include <renderer/Animator.hpp>
#include <renderer/Animation.hpp>
#include <renderer/Skeleton.hpp>

class AnimatedModel : public ModelBase {
public:
    struct Attachment {
        ModelBase* model;
        std::string joint_name;
        glm::vec3 offset_position = glm::vec3(0.0f);
        glm::vec3 offset_rotation = glm::vec3(0.0f);
        glm::vec3 offset_scale = glm::vec3(1.0f);
    };

private:
    Skeleton m_skeleton;
    Animator m_animator;
    std::vector<Animation*> m_animations;
    std::vector<Attachment> m_attachments;

public:
    AnimatedModel(const char* model_path, Shader* shader = nullptr) 
         : m_animator(&m_skeleton) {
            
        if (shader) {
            set_shader(shader);
        }

        _load_model(model_path, 
            aiProcess_JoinIdenticalVertices | 
            aiProcess_Triangulate | 
            aiProcess_GenUVCoords |
            aiProcess_LimitBoneWeights |
            aiProcess_GenNormals
        );

        // First process the skeleton if any mesh has bones
        for (unsigned int i = 0; i < num_meshes; ++i) {
            aiMesh* mesh = m_scene->mMeshes[i];
            if (mesh->HasBones()) {
                m_skeleton.init_from_mesh(mesh, m_scene);
                break;
            }
        }

        // Process meshes and materials
        for (unsigned int i = 0; i < num_meshes; ++i) {
            aiMesh* mesh = m_scene->mMeshes[i];
            _process_mesh(mesh, i);
            _process_material(m_scene->mMaterials[mesh->mMaterialIndex], i);
        }

        // Load animations if they exist
        if (m_scene->HasAnimations()) {
            for (unsigned int i = 0; i < m_scene->mNumAnimations; i++) {
                Animation* anim = Animation::from_assimp_animation(m_scene->mAnimations[i], m_scene);
                if (anim) {
                    m_animations.push_back(anim);
                    Log::log_success("(" + std::string(model_path) + ") Loaded animation " + std::to_string(i), __FILE__, __LINE__);
                }
            }
        }

        Log::log_success("(" + std::string(model_path) + ") Animated Model loaded successfully with " + std::to_string(num_meshes) + " meshes", 
            __FILE__, __LINE__);
    }

    ~AnimatedModel() {
        for (auto anim : m_animations) {
            delete anim;
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

    void reset_to_bind_pose() {
        if (m_skeleton.has_joints()) {
            m_skeleton.reset_to_bind_pose();
        }
    }

    Attachment* attach_to_joint(
        ModelBase* model,
        const std::string& joint_name,
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
                const Skeleton::Joint* joint = m_skeleton.get_joint_by_name(attachment.joint_name);
                if (joint) {
                    glm::mat4 global_transform = m_skeleton.get_global_transforms()[joint->id];
                    glm::mat4 attachment_transform = 
                        get_model_matrix() * 
                        global_transform *
                        Transform::create_model_matrix(
                            attachment.offset_position,
                            attachment.offset_rotation,
                            attachment.offset_scale
                        );

                    // Use the attached model's shader if it has one, otherwise use the current shader
                    Shader* model_shader = attachment.model->get_shader();
                    if (model_shader) {
                        model_shader->bind();
                        model_shader->set_uniform_mat4f("u_model", attachment_transform);
                        attachment.model->draw(*model_shader, false);
                    } else {
                        shader.set_uniform_mat4f("u_model", attachment_transform);
                        attachment.model->draw(shader, false);
                    }
                }
            }
        }
    }

private:
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

    VertexBufferLayout _create_vertex_buffer_layout() {
        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        layout.push<float>(2); // uv
        layout.push<int>(4);   // joint indices
        layout.push<float>(4); // weights
        return layout;
    }
};