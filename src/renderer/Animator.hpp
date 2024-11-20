#pragma once

#include <renderer/Animation.hpp>
#include <renderer/Skeleton.hpp>

#include <glm/glm.hpp>
#include <renderer/Animation.hpp>
#include <utils/Timer.h>

#include <glm/ext/quaternion_common.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class Animator {
private:
    Timer m_timer;
    float m_time_of_prev_frame;
    float m_animation_time;
    Animation* m_current_animation;
    Skeleton* m_skeleton;
    bool m_should_repeat = true;
    bool m_should_finish = false;
    float m_duration_s = 1.0f;
    bool m_should_play_backwards = false;

    void _step_time() {
        float current_time = float(m_timer.GetTime()) / 1000000.0f;  // Convert microseconds to seconds
        float delta_time = current_time - m_time_of_prev_frame;
        m_animation_time += delta_time;
        
        if (m_current_animation) {
            if (m_animation_time > m_duration_s) {
                
                if (!m_should_repeat) {
                    m_animation_time = m_duration_s;
                    // set_animation(nullptr);
                    return;
                }

                m_animation_time = fmod(m_animation_time, m_duration_s);
                if (m_animation_time < 0) {  // fmod can return negative values
                    m_animation_time += m_duration_s;
                }
            }
        }
        
        m_time_of_prev_frame = current_time;
    }

public:
    Animator() {
        init(nullptr);
    }
    
    Animator(Skeleton* skeleton) {
        init(skeleton);
    }

    ~Animator() {}

    void init(Skeleton* skeleton) {
        m_time_of_prev_frame = 0.0f;
        m_animation_time = 0.0f;
        m_current_animation = nullptr;
        m_skeleton = skeleton;
        m_time_of_prev_frame = float(m_timer.GetTime()) / 1000000.0f;
    }

    void update() {
        if (m_current_animation == nullptr) {
            return;
        }
        _step_time();
        step_animation();
    }

    float portion_complete() const {
        if (m_current_animation == nullptr) {
            return 1.0f;
        }
        // return m_animation_time / m_current_animation->get_duration();
        return m_animation_time / m_duration_s;
    }

    void set_animation(Animation* animation, const float& duration_s = 1.0f, const bool& should_repeat = true, const bool& should_finish = false, const bool& should_play_backwards = false) {
        m_current_animation = animation;
        m_animation_time = 0.0f;
        m_should_repeat = should_repeat;
        m_should_finish = should_finish;
        m_should_play_backwards = should_play_backwards;
        m_duration_s = duration_s;
        m_time_of_prev_frame = float(m_timer.GetTime()) / 1000000.0f;  // Convert microseconds to seconds
    }

    
    void replay() {
        m_animation_time = 0;
    }


    bool should_finish() const { return m_should_finish; }

    Animation* get_current_animation() const { return m_current_animation; }

    void step_animation() {
        if (!m_current_animation || !m_skeleton) return;

        const auto& frames = m_current_animation->get_key_frames();
        
        // Find the closest frame to current time
        KeyFrame* current_frame = nullptr;
        float smallest_delta = 999999.0f;  // Large enough value for animation timestamps
        
        for (size_t i = 0; i < frames.size(); ++i) {
            const auto& frame = frames[i];
            // float delta_time = std::abs(m_animation_time - frame.get_time_stamp());
            float animation_time = m_animation_time;
            if (m_should_play_backwards) {
                animation_time = m_duration_s - m_animation_time;
            }
            float delta_time = std::abs(animation_time - (m_duration_s * (frame.get_time_stamp() / m_current_animation->get_duration())));
            if (delta_time < smallest_delta) {
                smallest_delta = delta_time;
                current_frame = const_cast<KeyFrame*>(&frame);
            }
        }

        // Apply the frame's transforms directly
        if (current_frame) {
            for (const auto& joint : m_skeleton->get_joints()) {
                const glm::mat4* transform = current_frame->get_transform(joint.name);
                if (transform) {
                    if (joint.id == m_skeleton->get_root_joint()->id) {
                        // keep only the vertical motion for root joints
                        glm::mat4 root_transform = *transform;
                        root_transform[3].x = joint.local_bind_transform[3].x;
                        root_transform[3].z = joint.local_bind_transform[3].z;
                        m_skeleton->update_joint_pose(joint.id, root_transform);
                    } else {
                        m_skeleton->update_joint_pose(joint.id, *transform);
                    }
                }
            }
        }

        // Update the Transform matrix after updating the current pose
        m_skeleton->update_joint_transforms();
    }

#pragma region interpolation broken
    // void step_animation() {
    //     if (!m_current_animation || !m_skeleton) return;

    //     const auto& frames = m_current_animation->get_key_frames();
        
    //     KeyFrame* prev_frame = nullptr;
    //     KeyFrame* next_frame = nullptr;
        
    //     // Find surrounding frames
    //     for (size_t i = 0; i < frames.size(); ++i) {
    //         const auto& frame = frames[i];
    //         float delta_time = m_animation_time - frame.get_time_stamp();
    //         if (delta_time >= 0) {
    //             if (prev_frame == nullptr || delta_time < prev_frame->get_time_stamp() - m_animation_time) {
    //                 prev_frame = const_cast<KeyFrame*>(&frame);
    //             }
    //         } else if (delta_time < 0) {
    //             if (next_frame == nullptr || delta_time > next_frame->get_time_stamp() - m_animation_time) {
    //                 next_frame = const_cast<KeyFrame*>(&frame);
    //             }
    //         }
    //     }

    //     // Interpolate between frames
    //     if (prev_frame && next_frame) {
    //         float total_time = next_frame->get_time_stamp() - prev_frame->get_time_stamp();
    //         float t = (m_animation_time - prev_frame->get_time_stamp()) / total_time;
            
    //         // For each joint in the skeleton
    //         for (const auto& joint : m_skeleton->get_joints()) {
    //             const glm::mat4* prev_transform = prev_frame->get_transform(joint.name);
    //             const glm::mat4* next_transform = next_frame->get_transform(joint.name);
                
    //             if (prev_transform && next_transform) {
    //                 // Decompose and interpolate
    //                 glm::vec3 prev_pos, next_pos, prev_scale, next_scale;
    //                 glm::quat prev_rot, next_rot;
    //                 glm::vec3 skew;
    //                 glm::vec4 perspective;
                    
    //                 glm::decompose(*prev_transform, prev_scale, prev_rot, prev_pos, skew, perspective);
    //                 glm::decompose(*next_transform, next_scale, next_rot, next_pos, skew, perspective);

    //                 glm::vec3 pos = glm::mix(prev_pos, next_pos, t);
    //                 glm::quat rot = glm::slerp(prev_rot, next_rot, t);
    //                 glm::vec3 scale = glm::mix(prev_scale, next_scale, t);

    //                 glm::mat4 transform = 
    //                     glm::translate(glm::mat4(1.0f), pos) *
    //                     glm::mat4_cast(rot) *
    //                     glm::scale(glm::mat4(1.0f), scale);

    //                 m_skeleton->update_joint_pose(joint.id, transform);
    //             }
    //         }
    //     }

    //     // Update the Transform matrix after updating the current pose
    //     m_skeleton->update_joint_transforms();
    // }
#pragma endregion
};
