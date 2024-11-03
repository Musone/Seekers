#pragma once

#include "ecs/Registry.hpp"
#include <iostream>

class TutorialSystem {
public:
    bool w_pressed = false;
    bool a_pressed = false;
    bool s_pressed = false;
    bool d_pressed = false;
    bool space_pressed = false;
    bool mouse_button_left_pressed = false;
    bool movement_done = false;
    bool tutorial_done = false;

    static TutorialSystem& get_instance() {
        static TutorialSystem instance;
        return instance;
    }

    void handle_tutorial_per_frame() {
        if (tutorial_done) {
            return;
        }

        Registry& registry = Registry::get_instance();
        InputState& input_state = registry.input_state;

        handle_movement_check(input_state);
        
        if (w_pressed && a_pressed && s_pressed && d_pressed && !space_pressed && !mouse_button_left_pressed) {
            handle_dodge_check(input_state);
        }

        if (w_pressed && a_pressed && s_pressed && d_pressed && space_pressed && !mouse_button_left_pressed) {
            handle_attack_check(input_state);
        }

        if (w_pressed && a_pressed && s_pressed && d_pressed && space_pressed && mouse_button_left_pressed && !tutorial_done) {
            // reset_tutorial_flags();
            std::cout << "Tutorial Done" << std::endl;
            tutorial_done = true;
        }
    }

    void load_tutorial() {
        // load movement png here (png/wasd.png)
        std::cout << "Tutorial Loaded" << std::endl;
    }

    void handle_movement_check(const InputState& input_state) {
        if (input_state.w_down && !w_pressed) {
            w_pressed = true;
            std::cout << "W Pressed" << std::endl;
        }
        if (input_state.a_down && !a_pressed) {
            a_pressed = true;
            std::cout << "A Pressed" << std::endl;
        }
        if (input_state.s_down && !s_pressed) {
            s_pressed = true;
            std::cout << "S Pressed" << std::endl;
        }
        if (input_state.d_down && !d_pressed) {
            d_pressed = true;
            std::cout << "D Pressed" << std::endl;
        }
        if (w_pressed && a_pressed && s_pressed && d_pressed && !movement_done) {
            // remove movement png & load dodge png here (png/spacebar.png)
            std::cout << "Movement Done" << std::endl;
            movement_done = true;
        }
    }

    void handle_dodge_check(const InputState& input_state) {
        if (input_state.space_down && !space_pressed) {
            space_pressed = true;
            std::cout << "Space Pressed" << std::endl;
        }
        if (space_pressed && !mouse_button_left_pressed) {
            // remove dodge png & load attack png here (png/leftclick.png)
            std::cout << "Dodge Done" << std::endl;
            handle_attack_check(input_state);
        }
    }

    void handle_attack_check(const InputState& input_state) {
        if (input_state.mouse_button_left_down && !mouse_button_left_pressed) {
            mouse_button_left_pressed = true;
            std::cout << "Left Click Pressed" << std::endl;
        }
        if (mouse_button_left_pressed) {
            // remove attack png here
        }
    }

    void reset_tutorial_flags() {
        w_pressed = false;
        a_pressed = false;
        s_pressed = false;
        d_pressed = false;
        space_pressed = false;
        mouse_button_left_pressed = false;
    }

private:
};