#define NOMINMAX

#include <utils/Log.hpp>
#include <app/Application.hpp>
#include <Testing.hpp>

#include <ft2build.h>
#include <freetype/freetype.h>

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
        app.run_game_loop();
        // Testing::try_assimp();
    } catch (const std::exception& e) {
        const std::string message = std::string(e.what());
        Log::log_error_and_terminate(message, __FILE__, __LINE__);
    }

    return 0;
}

// int main() {
//     FT_Library ft;
//     if (FT_Init_FreeType(&ft)) {
//         std::cerr << "ERROR::FREETYTPE: Could not init FreeType Library" << std::endl;
//         return 1;
//     }
//
//     // Load a font face
//     FT_Face face;
//     if (FT_New_Face(ft, "fonts/Cano-VGMwz.ttf", 0, &face)) {
//         std::cerr << "ERROR::FREETYTPE: Could not open font" << std::endl;
//         return 1;
//     }
//
//     //  Once we've loaded the face, we should define the pixel font size we'd like to extract from this face:
//     FT_Set_Pixel_Sizes(face, 0, 48);
//
//     // A FreeType face hosts a collection of glyphs. We can set one of those glyphs as the active glyph by calling FT_Load_Char. Here we choose to load the character glyph 'X':
//     if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) {
//         std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//         return -1;
//     }
//
//     face->glyph->bitmap;
//
//     // Clean up
//     FT_Done_Face(face);
//     FT_Done_FreeType(ft);
//
//     std::cout << "FreeType initialized successfully!" << std::endl;
//     return 0;
// }