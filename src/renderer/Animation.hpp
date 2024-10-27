#pragma once

#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/anim.h>
#include <assimp/scene.h>

#include <unordered_map>
#include <string>
#include <set>

class KeyFrame {
private:
    float m_time_stamp;
    std::unordered_map<std::string, glm::mat4> m_pose;

public:
    KeyFrame(float time_stamp = 0.0f) : m_time_stamp(time_stamp) {}
    ~KeyFrame() = default;

    void add_transform(const std::string& joint_name, const glm::mat4& transform) {
        m_pose[joint_name] = transform;
    }

    const glm::mat4* get_transform(const std::string& joint_name) const {
        auto it = m_pose.find(joint_name);
        return it != m_pose.end() ? &it->second : nullptr;
    }

    float get_time_stamp() const { return m_time_stamp; }
};

class Animation {
private:
    float m_duration_seconds;
    std::vector<KeyFrame> m_key_frames;

    static aiVector3D _find_position(aiNodeAnim* channel, float time) {
        for (unsigned int i = 0; i < channel->mNumPositionKeys - 1; i++) {
            if (channel->mPositionKeys[i + 1].mTime >= time) {
                float t = (time - channel->mPositionKeys[i].mTime) / 
                         (channel->mPositionKeys[i + 1].mTime - channel->mPositionKeys[i].mTime);
                return channel->mPositionKeys[i].mValue * (1-t) + channel->mPositionKeys[i+1].mValue * t;
            }
        }
        return channel->mPositionKeys[channel->mNumPositionKeys - 1].mValue;
    }

    static aiQuaternion _find_rotation(aiNodeAnim* channel, float time) {
        // Find closest rotation key
        for (unsigned int i = 0; i < channel->mNumRotationKeys; i++) {
            if (channel->mRotationKeys[i].mTime >= time) {
                return channel->mRotationKeys[i].mValue;
            }
        }
        // If we didn't find a key, return the last one
        return channel->mRotationKeys[channel->mNumRotationKeys - 1].mValue;
    }

    // Quaternion interpolation broken...
    // static aiQuaternion _find_rotation(aiNodeAnim* channel, float time) {
    //     for (unsigned int i = 0; i < channel->mNumRotationKeys - 1; i++) {
    //         if (channel->mRotationKeys[i + 1].mTime >= time) {
    //             float t = (time - channel->mRotationKeys[i].mTime) / 
    //                      (channel->mRotationKeys[i + 1].mTime - channel->mRotationKeys[i].mTime);
    //             aiQuaternion out;
    //             aiQuaternion::Interpolate(out, 
    //                 channel->mRotationKeys[i].mValue,
    //                 channel->mRotationKeys[i + 1].mValue, t);
    //             return out;
    //         }
    //     }
    //     return channel->mRotationKeys[channel->mNumRotationKeys - 1].mValue;
    // }

    static aiVector3D _find_scaling(aiNodeAnim* channel, float time) {
        for (unsigned int i = 0; i < channel->mNumScalingKeys - 1; i++) {
            if (channel->mScalingKeys[i + 1].mTime >= time) {
                float t = (time - channel->mScalingKeys[i].mTime) / 
                         (channel->mScalingKeys[i + 1].mTime - channel->mScalingKeys[i].mTime);
                return channel->mScalingKeys[i].mValue * (1-t) + channel->mScalingKeys[i+1].mValue * t;
            }
        }
        return channel->mScalingKeys[channel->mNumScalingKeys - 1].mValue;
    }

public:
    Animation(std::vector<KeyFrame>&& key_frames, const float& duration_seconds) :
        m_key_frames(std::move(key_frames)),
        m_duration_seconds(duration_seconds) {}

    ~Animation() = default;

    float get_duration() const { return m_duration_seconds; }
    const std::vector<KeyFrame>& get_key_frames() const { return m_key_frames; }
    size_t get_frame_count() const { return m_key_frames.size(); }

    static Animation* from_assimp_animation(const aiAnimation* anim, const aiScene* scene) {
        float duration = anim->mDuration / anim->mTicksPerSecond;
        
        // Find all unique timestamps
        std::set<float> timestamps;
        for (unsigned int channel_idx = 0; channel_idx < anim->mNumChannels; channel_idx++) {
            aiNodeAnim* channel = anim->mChannels[channel_idx];
            for (unsigned int i = 0; i < channel->mNumPositionKeys; i++) {
                timestamps.insert(channel->mPositionKeys[i].mTime / anim->mTicksPerSecond);
            }
        }

        // Create keyframes vector
        std::vector<KeyFrame> keyframes;
        keyframes.reserve(timestamps.size());

        for (float time : timestamps) {
            keyframes.emplace_back(time);
            
            // For each channel at this timestamp
            for (unsigned int channel_idx = 0; channel_idx < anim->mNumChannels; channel_idx++) {
                aiNodeAnim* channel = anim->mChannels[channel_idx];
                std::string joint_name = channel->mNodeName.C_Str();

                // Find position, rotation, scale keys at this time
                aiVector3D position = _find_position(channel, time * anim->mTicksPerSecond);
                aiQuaternion rotation = _find_rotation(channel, time * anim->mTicksPerSecond);
                aiVector3D scaling = _find_scaling(channel, time * anim->mTicksPerSecond);

                // Convert to matrix
                glm::mat4 transform = 
                    glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z)) *
                    glm::mat4_cast(glm::quat(rotation.w, rotation.x, rotation.y, rotation.z)) *
                    glm::scale(glm::mat4(1.0f), glm::vec3(scaling.x, scaling.y, scaling.z));

                keyframes.back().add_transform(joint_name, transform);
            }
        }

        return new Animation(std::move(keyframes), duration);
    }
};