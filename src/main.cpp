#include <utils/Log.hpp>
#include <app/Application.hpp>

int main(void) {
    try {
        Application app;
<<<<<<< HEAD
        // app.run_demo_basic();
        app.run_demo_texture();
=======
        app.run_test_loop();
>>>>>>> c40bf06 (Refactored renderer. Created a test demo of its usage in application.hpp.)
    } catch (const std::exception& e) {
        const std::string message = std::string(e.what());
        Log::log_error_and_terminate(message, __FILE__, __LINE__);
    }

    return 0;
}
