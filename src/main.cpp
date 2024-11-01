#include <utils/Log.hpp>
#include <app/Application.hpp>
#include <Testing.hpp>

int main(void) {
    try {
        // app.run_demo_world();
        Application app;
        app.run_game_loop();
        // Testing::try_assimp();
    } catch (const std::exception& e) {
        const std::string message = std::string(e.what());
        Log::log_error_and_terminate(message, __FILE__, __LINE__);
    }

    return 0;
}
