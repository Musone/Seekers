#pragma once

#include <iostream>

class TutorialSystem {
public:
    enum class TUTORIAL_STATE {
        NOTHING_DONE = 0,
        MOVEMENTS_DONE = NOTHING_DONE + 1,
        AIM_DONE = MOVEMENTS_DONE + 1,
        DODGE_DONE = AIM_DONE + 1,
        ATTACK_DONE = DODGE_DONE + 1,
        TUTORIAL_DONE = ATTACK_DONE
    };

    TUTORIAL_STATE state = TUTORIAL_STATE::NOTHING_DONE;

    static TutorialSystem& get_instance() {
        static TutorialSystem instance;
        return instance;
    }

    inline static void pass_movements() {
        TutorialSystem& instance = TutorialSystem::get_instance();
        if (instance.state == TUTORIAL_STATE::NOTHING_DONE) {
            instance.state = TUTORIAL_STATE::MOVEMENTS_DONE;
        }
    }

    inline static void pass_aim() {
        TutorialSystem& instance = TutorialSystem::get_instance();
        if (instance.state == TUTORIAL_STATE::MOVEMENTS_DONE) {
            instance.state = TUTORIAL_STATE::AIM_DONE;
        }
    }

    inline static void pass_dodge() {
        TutorialSystem& instance = TutorialSystem::get_instance();
        if (instance.state == TUTORIAL_STATE::AIM_DONE) {
            instance.state = TUTORIAL_STATE::DODGE_DONE;
        }
    }

    inline static void pass_attack() {
        TutorialSystem& instance = TutorialSystem::get_instance();
        if (instance.state == TUTORIAL_STATE::DODGE_DONE) {
            instance.state = TUTORIAL_STATE::ATTACK_DONE;
        }
    }

private:
};