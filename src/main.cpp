#define NOMINMAX

#include <utils/Log.hpp>
#include <app/Application.hpp>
#include <Testing.hpp>

#if __APPLE__
#else
extern "C" 
{
  __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
};
#endif // !__APPLE__

int main(void) {
    try {
        Application app;
        // app.run_demo_world();
        app.run_game_loop();
        // Testing::try_assimp();
    } catch (const std::exception& e) {
        const std::string message = std::string(e.what());
        Log::log_error_and_terminate(message, __FILE__, __LINE__);
    }

    return 0; 
}
