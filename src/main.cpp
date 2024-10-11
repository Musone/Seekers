#include <utils/Log.hpp>
#include <app/Application.hpp>

int main(void) {
    try {
        Application app;
        app.run_test_loop();
    } catch (const std::exception& e) {
        const std::string message = std::string(e.what());
        Log::log_error_and_terminate(message, __FILE__, __LINE__);
    }

    return 0;
}
