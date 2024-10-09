#pragma once

#include <stdexcept>

namespace InputManager {
    void on_key_pressed(int key, int scancode, int action, int mods) {
        throw std::runtime_error("on_key_pressed not implemented");
    }

    void on_mouse_button_pressed(int button, int action, int mods) {
        throw std::runtime_error("on_mouse_button_pressed not implemented");
    }

    void on_mouse_move(int x, int y) {
        throw std::runtime_error("on_mouse_move not implemented");
    }
}