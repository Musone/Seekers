#include <ecs/Registry.hpp>
#include <utils/FileSystem.hpp>
#include <utils/Log.hpp>
#include <renderer/Renderer.hpp>

#include <iostream>

int main(void) {
    try {
        Renderer renderer;
        renderer.init();
        /* Loop until the user closes the window */
        while (!renderer.is_terminated()) {
            renderer.draw();
        };
    } catch (const std::exception& e) {
        const std::string message = std::string(e.what());
        Log::log_error_and_terminate(message);
    }


    return 0;
}
