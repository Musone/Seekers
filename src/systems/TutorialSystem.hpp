#pragma once

#include "ecs/Registry.hpp"
#include <iostream>

class TutorialSystem {
public:
    static TutorialSystem& get_instance() {
        static TutorialSystem instance;
        return instance;
    }

    void handle_tutorial_per_frame() {
        Registry& registry = Registry::get_instance();
        InputState& input_state = registry.input_state;

        // Check button presses
        handle_button_check(input_state);

        // Check if the tutorial is complete
        if (tutorial_check()) {
            std::cout << "Tutorial Finished" << std::endl;
            reset_tutorial_flags(); // Reset the flags for potential reuse
        }
    }

private:
    bool w_pressed = false;
    bool a_pressed = false;
    bool s_pressed = false;
    bool d_pressed = false;
    bool space_pressed = false;
    bool mouse_button_left_pressed = false;

    void handle_button_check(const InputState& input_state) {
        // Update pressed states based on current input
        if (input_state.w_down && !w_pressed) {
            w_pressed = true;
            std::cout << "W Finished" << std::endl;
        }
        if (input_state.a_down && !a_pressed) {
            a_pressed = true;
            std::cout << "A Finished" << std::endl;
        }
        if (input_state.s_down && !s_pressed) {
            s_pressed = true;
            std::cout << "S Finished" << std::endl;
        }
        if (input_state.d_down && !d_pressed) {
            d_pressed = true;
            std::cout << "D Finished" << std::endl;
        }
        if (input_state.space_down && !space_pressed) {
            space_pressed = true;
            std::cout << "Space Finished" << std::endl;
        }
        if (input_state.mouse_button_left_down && !mouse_button_left_pressed) {
            mouse_button_left_pressed = true;
            std::cout << "Left Click Finished" << std::endl;
        }
    }

    bool tutorial_check() {
        return w_pressed && a_pressed && s_pressed && d_pressed &&
            space_pressed && mouse_button_left_pressed;
    }

    void reset_tutorial_flags() {
        w_pressed = false;
        a_pressed = false;
        s_pressed = false;
        d_pressed = false;
        space_pressed = false;
        mouse_button_left_pressed = false;
    }
};
