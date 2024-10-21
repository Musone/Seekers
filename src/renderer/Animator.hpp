#pragma once

#include <renderer/Animation.hpp>
#include <utils/Timer.h>

class Animator {
    Timer m_timer;
    float m_time_of_prev_frame;
    float m_animation_time;

    Animation* m_current_animation;
public:
    Animator() :
    m_time_of_prev_frame(0.0f) {

    }

    ~Animator() {

    }

    void update() {
        if (m_current_animation == nullptr) {
            return;
        }
        _step_time();

    }

    void step_animation() {
        KeyFrame* frames = m_current_animation->get_key_frames();
        int n_frames = m_current_animation->get_frame_count();

        KeyFrame* prev_frame = nullptr;
        KeyFrame* next_frame = nullptr;
        for (int i = 0; i < n_frames; ++i) {
            auto& frame = frames[i];
            float delta_time = m_animation_time - frame.get_time_stamp();
            if (delta_time >= 0) {
                if (prev_frame == nullptr || delta_time < prev_frame->get_time_stamp() - m_animation_time) {
                    prev_frame = &frame;
                }
            } else if (delta_time < 0) {
                if (next_frame == nullptr || delta_time > next_frame->get_time_stamp() - m_animation_time) {
                    next_frame = &frame;
                }
            }
        }
        
    }

private:
    void _step_time() {
        float current_time = float(m_timer.GetTime());
        float delta_time = current_time - m_time_of_prev_frame;
        m_animation_time += delta_time;
        while (m_animation_time > m_current_animation->get_duration()) {
            m_animation_time = 0;
        }
        m_time_of_prev_frame = current_time;
    }
};