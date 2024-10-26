#pragma once

#include <renderer/GLUtils.hpp>
#include <glm/glm.hpp>
#include <renderer/Texture2D.hpp>
#include <renderer/Shader.hpp>
#include <renderer/Mesh.hpp>
#include <renderer/Camera.hpp>
#include <utils/Transform.hpp>
// include joint and animator later.
#include <renderer/Joint.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
private:
    struct JointInfo {
        std::string name;
        int id;
        glm::mat4 inverse_bind_matrix;
        glm::mat4 local_bind_transform;
        std::vector<JointInfo*> children;
    };

    // Mesh* m_mesh;
    // Texture2D* m_texture;
    // Shader* m_shader;
    
    JointInfo* m_root_joint = nullptr;
    std::vector<glm::mat4> m_inverse_bind_matrices;
    std::vector<glm::mat4> m_joint_transforms;
    std::vector<glm::mat4> m_current_pose;
    
    glm::vec3 m_position;
    glm::vec3 m_rotation;
    glm::vec3 m_scale;

    Assimp::Importer m_importer;
    const aiScene* m_scene = nullptr;
    aiNode* m_root_node = nullptr;

    std::vector<JointInfo> m_joints;
    std::unordered_map<std::string, int> m_joint_name_to_id;

public:
    std::vector<Mesh> mesh_list;
    std::vector<Texture2D> texture_list;
    unsigned int num_meshes;

    Model(const char* model_path) : 
        m_position(0.0f),
        m_rotation(0.0f),
        m_scale(1.0f) {
        _load_model(model_path);
    }

    Model(Mesh* mesh, Texture2D* texture, Shader* shader, Joint* root_joint, int joint_count) :
        // m_mesh(mesh),
        // m_texture(texture),
        // m_shader(shader),
        // m_root_joint(root_joint),
        // m_joint_count(joint_count),
        m_position(glm::vec3(0.0f)),
        m_rotation(glm::vec3(0.0f)),
        m_scale(glm::vec3(1.0f))
        // m_joint_transforms(nullptr)
         {
        // if (m_joint_count > 0 && m_root_joint != nullptr) {
            // m_root_joint->set_inverse_bind_transform(glm::mat4(1.0));
            // m_joint_transforms = new glm::mat4[joint_count];
        // }
        // create an animator
    }
    
    ~Model() {
        // delete[] m_joint_transforms;
    }

    // Existing getters
    glm::mat4 get_model_matrix() const { 
        return Transform::create_model_matrix(
            m_position,
            m_rotation,
            m_scale
        ); 
    }

    unsigned int get_face_count() const { 
        throw new std::runtime_error("This has been deprecated."); 
    }

    void draw(Shader& shader) const {
        shader.bind();
        
        // Set model transform
        shader.set_uniform_mat4f("u_model", get_model_matrix());

        // Set animation data if available
        if (!m_joints.empty()) {
            shader.set_uniform_mat4f_array("u_joint_transforms", m_joint_transforms.data()[0], m_joints.size());
            shader.set_uniform_mat4f_array("u_inv_bind", m_inverse_bind_matrices.data()[0], m_joints.size());
        }

        // Draw each mesh
        for (unsigned int i = 0; i < num_meshes; i++) {
            // Bind mesh data
            mesh_list[i].bind();
            
            // Bind texture if available
            if (mesh_list[i].m_texture) {
                shader.set_uniform_1i("u_texture", mesh_list[i].m_texture->bind(1));
            }

            // Draw
            GL_Call(glDrawElements(GL_TRIANGLES, mesh_list[i].get_face_count(), GL_UNSIGNED_INT, 0));
            
            // Unbind mesh data
            mesh_list[i].unbind();
        }
    }

#pragma region comprehensive getters and setters
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
#pragma endregion

private:
    void _load_model(const char* model_path) {
        m_scene = m_importer.ReadFile(model_path, 
            aiProcess_JoinIdenticalVertices | 
            aiProcess_Triangulate | 
            // aiProcess_FlipUVs |
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
                _process_skeleton(mesh, m_scene);
                break;  // Only need to process skeleton once
            }
        }

        // Then process all meshes
        for (unsigned int i = 0; i < num_meshes; ++i) {
            aiMesh* mesh = m_scene->mMeshes[i];
            _process_mesh(mesh, i);
            _process_material(m_scene->mMaterials[mesh->mMaterialIndex], i);
        }

        // TODO: LOAD ANIMATIONS
        // if (m_scene->HasAnimations()) {
        //     _process_animations(m_scene);
        // }
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

            // 4 weights (as vec4)
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

            size_t base_offset = i * (3 + 3 + 2 + 4 + 4); // pos norm texcoords 4joints 4weights
            size_t joint_offset = base_offset + 8;
            size_t weight_offset = joint_offset + 4;

            float total_weight = 0.0f;

            for (size_t j = 0; j < std::min(size_t(4), weights.size()); j++) {
                vertices[joint_offset + j] = weights[j].first;
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
            }
            mesh_list[mesh_index].set_texture(texture);
        }
    }

    void _process_skeleton(aiMesh* mesh, const aiScene* scene) {
        m_joints.resize(mesh->mNumBones);
        m_joint_name_to_id.reserve(mesh->mNumBones);
        m_current_pose.resize(mesh->mNumBones, glm::mat4(1.0f));  // Initialize with identity matrices
        m_joint_transforms.resize(mesh->mNumBones); 

        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            aiBone* bone = mesh->mBones[i];
            m_joint_name_to_id[bone->mName.C_Str()] = i;
            
            aiMatrix4x4 offset = bone->mOffsetMatrix;
            m_joints[i].inverse_bind_matrix = glm::mat4(
                offset.a1, offset.b1, offset.c1, offset.d1,
                offset.a2, offset.b2, offset.c2, offset.d2,
                offset.a3, offset.b3, offset.c3, offset.d3,
                offset.a4, offset.b4, offset.c4, offset.d4
            );
            
            m_joints[i].name = bone->mName.C_Str();
            m_joints[i].id = i;
        }

        // Initialize inverse bind matrices once
        m_inverse_bind_matrices.reserve(m_joints.size());
        for (const auto& joint : m_joints) {
            m_inverse_bind_matrices.push_back(joint.inverse_bind_matrix);
        }

        _build_skeleton_hierarchy(scene->mRootNode, nullptr);
    }

    void _build_skeleton_hierarchy(aiNode* node, JointInfo* parent) {
        auto it = m_joint_name_to_id.find(node->mName.C_Str());
        JointInfo* current_joint = nullptr;

        if (it != m_joint_name_to_id.end()) {
            current_joint = &m_joints[it->second];
            
            aiMatrix4x4 transform = node->mTransformation;
            current_joint->local_bind_transform = glm::mat4(
                transform.a1, transform.b1, transform.c1, transform.d1,
                transform.a2, transform.b2, transform.c2, transform.d2,
                transform.a3, transform.b3, transform.c3, transform.d3,
                transform.a4, transform.b4, transform.c4, transform.d4
            );

            // If this joint has no parent, it's the root
            if (!parent) {
                m_root_joint = current_joint;
            } else {
                parent->children.push_back(current_joint);
            }
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            _build_skeleton_hierarchy(node->mChildren[i], current_joint ? current_joint : parent);
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