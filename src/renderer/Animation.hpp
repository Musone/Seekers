#pragma once

#include <renderer/KeyFrame.hpp>

class Animation {
    float m_duration_seconds;
    KeyFrame* m_key_frames;
    int m_key_frame_count;
public:
    Animation(KeyFrame* key_frames, const int& key_frame_count, const float& duration_seconds) :
    m_key_frames(key_frames),
    m_key_frame_count(key_frame_count),
    m_duration_seconds(duration_seconds) {

    }

    ~Animation() {}

    float get_duration() const {
        return m_duration_seconds;
    }

    KeyFrame* get_key_frames() const {
        return m_key_frames;
    }

    int get_frame_count() const {
        return m_key_frame_count;
    }
};