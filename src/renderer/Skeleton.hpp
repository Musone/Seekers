#pragma once

#include <utils/Log.hpp>

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <stack>

class Skeleton {
public:
    struct Joint {
        std::string name;
        int id;
        glm::mat4 inverse_bind_matrix;
        glm::mat4 local_bind_transform;
        std::vector<Joint*> children;
    };

private:
    std::vector<Joint> m_joints;
    std::unordered_map<std::string, int> m_joint_name_to_id;
    std::vector<glm::mat4> m_inverse_bind_matrices;
    std::vector<glm::mat4> m_joint_transforms;
    std::vector<glm::mat4> m_global_transforms;
    std::vector<glm::mat4> m_current_pose;
    Joint* m_root_joint = nullptr;
    bool m_is_initialized = false;

public:
    Skeleton() = default;

    Skeleton(const Skeleton& original) {
        m_joint_name_to_id = original.m_joint_name_to_id;
        m_inverse_bind_matrices = original.m_inverse_bind_matrices;
        m_joint_transforms = original.m_joint_transforms;
        m_global_transforms = original.m_global_transforms;
        m_current_pose = original.m_current_pose;
        m_is_initialized = original.m_is_initialized;
        
        m_joints.reserve(original.m_joints.size());
        for (const auto& joint : original.m_joints) {
            m_joints.push_back({
                joint.name,
                joint.id,
                joint.inverse_bind_matrix,
                joint.local_bind_transform,
            });

            Joint& current = m_joints.back();
            for (const auto& child : joint.children) {
                current.children.push_back(&m_joints[child->id]);
            }
        }

        m_root_joint = &m_joints[original.m_root_joint->id];
    };
    
    void init_from_bones(const std::vector<aiBone*>& bones, const aiScene* scene) {
        if (m_is_initialized) {
            Log::log_warning("Skeleton already initialized, skipping initialization", __FILE__, __LINE__);
            return;
        }

        if (bones.empty()) {
            Log::log_warning("No bones provided, skipping skeleton initialization", __FILE__, __LINE__);
            return;
        }

        _process_bones(bones);
        _build_hierarchy(scene->mRootNode, nullptr);

        if (!m_root_joint) {
            Log::log_error_and_terminate("Failed to find root joint - critical error", __FILE__, __LINE__);
        }

        m_is_initialized = true;
        Log::log_success("Skeleton initialized successfully with " + 
            std::to_string(m_joints.size()) + " joints", __FILE__, __LINE__);
        
        reset_to_bind_pose();
    }

    const std::vector<glm::mat4>& get_joint_transforms() const { 
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Attempting to get transforms from uninitialized skeleton", __FILE__, __LINE__);
        }
        return m_joint_transforms; 
    }

    const std::vector<glm::mat4>& get_global_transforms() const { 
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Attempting to get transforms from uninitialized skeleton", __FILE__, __LINE__);
        }
        return m_global_transforms; 
    }

    const std::vector<glm::mat4>& get_inverse_bind_matrices() const { 
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Attempting to get inverse bind matrices from uninitialized skeleton", __FILE__, __LINE__);
        }
        return m_inverse_bind_matrices; 
    }

    bool has_joints() const { 
        return m_is_initialized && !m_joints.empty(); 
    }

    size_t get_joint_count() const { 
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Attempting to get joint count from uninitialized skeleton", __FILE__, __LINE__);
        }
        return m_joints.size(); 
    }

    const Joint* get_root_joint() const {
        if (!m_is_initialized) {
            Log::log_warning("Attempting to get root joint from uninitialized skeleton", __FILE__, __LINE__);
            return nullptr;
        }
        return m_root_joint;
    }

    const std::vector<Joint>& get_joints() { return m_joints; }

    const Joint* get_joint_by_name(const std::string& name) const {
        if (!m_is_initialized) {
            Log::log_warning("Attempting to get joint from uninitialized skeleton", __FILE__, __LINE__);
            return nullptr;
        }
        auto it = m_joint_name_to_id.find(name);
        if (it == m_joint_name_to_id.end()) {
            Log::log_warning("Joint '" + name + "' not found in skeleton", __FILE__, __LINE__);
            return nullptr;
        }
        return &m_joints[it->second];
    }

    const Joint* get_joint_by_id(int id) const {
        if (!m_is_initialized) {
            Log::log_warning("Attempting to get joint from uninitialized skeleton", __FILE__, __LINE__);
            return nullptr;
        }
        if (id < 0 || id >= static_cast<int>(m_joints.size())) {
            Log::log_warning("Joint ID " + std::to_string(id) + " out of range", __FILE__, __LINE__);
            return nullptr;
        }
        return &m_joints[id];
    }

    void update_joint_transforms() {
        if (!m_is_initialized) {
            Log::log_warning("Attempting to update transforms of uninitialized skeleton", __FILE__, __LINE__);
            return;
        }

        struct JointUpdate {
            Joint* joint;
            int parent_id;
        };

        std::stack<JointUpdate> update_stack;

        if (m_root_joint) {
            update_stack.push({m_root_joint, -1});
        }

        while (!update_stack.empty()) {
            auto current = update_stack.top();
            update_stack.pop();

            Joint* joint = current.joint;
            int parent_id = current.parent_id;

            // Calculate global transform
            if (parent_id > -1) {
                m_global_transforms[joint->id] = m_global_transforms[parent_id] * m_current_pose[joint->id];
            } else {
                m_global_transforms[joint->id] = m_current_pose[joint->id];
            }

            // Store final transform for skinning
            // Final = Global * Inverse Bind
            m_joint_transforms[joint->id] = m_global_transforms[joint->id] * joint->inverse_bind_matrix;

            // Add children to stack in reverse order
            for (auto it = joint->children.rbegin(); it != joint->children.rend(); ++it) {
                update_stack.push({*it, joint->id});
            }
        }
    }

    // Add method to update current pose
    void update_joint_pose(int joint_id, const glm::mat4& new_pose) {
        if (!m_is_initialized) {
            Log::log_warning("Attempting to update pose of uninitialized skeleton", __FILE__, __LINE__);
            return;
        }

        if (joint_id < 0 || joint_id >= static_cast<int>(m_joints.size())) {
            Log::log_warning("Invalid joint ID for pose update: " + std::to_string(joint_id), 
                __FILE__, __LINE__);
            return;
        }

        m_current_pose[joint_id] = new_pose;
        int ddwqdwq= 0;
        int ddwqdw11q= 0;
        int ddwqdwdsaq= 0;
        int ddwqq= 0;
    }

    // Add method to get current pose
    const std::vector<glm::mat4>& get_current_pose() const {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Attempting to get pose from uninitialized skeleton", 
                __FILE__, __LINE__);
        }
        return m_current_pose;
    }

    void reset_to_bind_pose() {
        if (!m_is_initialized) {
            Log::log_warning("Attempting to reset pose of uninitialized skeleton", __FILE__, __LINE__);
            return;
        }
        
        // Set current pose to local bind transforms
        for (const auto& joint : m_joints) {
            m_current_pose[joint.id] = joint.local_bind_transform;
        }
        
        update_joint_transforms();
        Log::log_success("Reset skeleton to bind pose", __FILE__, __LINE__);
    }

    void print_bones(const std::string& model_name) const {
        std::cout << "Model " << model_name << " has bones:\n";
        for (const auto& kv : m_joint_name_to_id) {
            std::cout << "  Bone " << kv.second << ": \"" << kv.first << "\"\n";
        }
        std::cout << "\n\n";
    }

private:

    void _process_bones(const std::vector<aiBone*>& bones) {
        m_joints.resize(bones.size());
        m_joint_name_to_id.reserve(bones.size());
        m_current_pose.resize(bones.size(), glm::mat4(1.0f));
        m_joint_transforms.resize(bones.size());
        m_global_transforms.resize(bones.size());

        for (size_t i = 0; i < bones.size(); i++) {
            aiBone* bone = bones[i];
            m_joint_name_to_id[bone->mName.C_Str()] = i;
            
            m_joints[i].inverse_bind_matrix = _convert_matrix(bone->mOffsetMatrix);
            m_joints[i].name = bone->mName.C_Str();
            m_joints[i].id = i;
        }

        m_inverse_bind_matrices.reserve(m_joints.size());
        for (const auto& joint : m_joints) {
            m_inverse_bind_matrices.push_back(joint.inverse_bind_matrix);
        }
    }

    void _build_hierarchy(aiNode* node, Joint* parent) {
        auto it = m_joint_name_to_id.find(node->mName.C_Str());
        Joint* current_joint = nullptr;

        if (it != m_joint_name_to_id.end()) {
            current_joint = &m_joints[it->second];
            current_joint->local_bind_transform = _convert_matrix(node->mTransformation);

            if (!parent) {
                m_root_joint = current_joint;
                Log::log_success("Found root joint: " + current_joint->name, __FILE__, __LINE__);
            } else {
                parent->children.push_back(current_joint);
                // Log::log_success("Added joint '" + current_joint->name + "' as child of '" + parent->name + "'", 
                    // __FILE__, __LINE__);
            }
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            _build_hierarchy(node->mChildren[i], current_joint ? current_joint : parent);
        }
    }

    static glm::mat4 _convert_matrix(const aiMatrix4x4& from) {
        return glm::mat4(
            from.a1, from.b1, from.c1, from.d1,
            from.a2, from.b2, from.c2, from.d2,
            from.a3, from.b3, from.c3, from.d3,
            from.a4, from.b4, from.c4, from.d4
        );
    }

    void _print_matrix(const glm::mat4& m) {
        for (int i = 0; i < 4; i++) {
            std::cout << "[ ";
            for (int j = 0; j < 4; j++) {
                std::cout << m[j][i] << " ";
            }
            std::cout << "]\n";
        }
        std::cout << "\n";
    }
};