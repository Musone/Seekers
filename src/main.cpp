#include <utils/Log.hpp>
#include <app/Application.hpp>
#include <Testing.hpp>

int main(void) {
    try {
        Application app;
        // app.run_demo_basic();
        // app.run_demo_texture();
        // app.run_demo_camera();
        // app.run_demo_world();
        // app.run_demo_obj_3d_model();
        // app.run_demo_callada_3d_model();
        Testing::do_stuff();
    } catch (const std::exception& e) {
        const std::string message = std::string(e.what());
        Log::log_error_and_terminate(message, __FILE__, __LINE__);
    }

    return 0;
}
