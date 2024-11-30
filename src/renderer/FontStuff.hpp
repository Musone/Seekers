#pragma once

#include <gl3w.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include <freetype/freetype.h>
#include FT_FREETYPE_H

// matrices
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <utils/Log.hpp>
#include <renderer/Shader.hpp>

#include <string>
#include <map>

#include <app/MapManager.hpp>
#include <renderer/GLUtils.hpp>

// Jakob: Created following the instructions at
// https://learnopengl.com/In-Practice/Text-Rendering
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
    char character;
};

class FontStuff {
private:
    FontStuff() = default;
    FontStuff(const FontStuff&) = delete;
    void operator=(const FontStuff&) = delete;
public:
    static FontStuff& get_instance() {
        static FontStuff instance;
        return instance;
    }

    unsigned int m_font_vao;
    unsigned int m_font_vbo;
    Shader m_font_shader;
    float font_default_size;

    std::unordered_map<char, Character> m_ftCharacters;

    bool font_init(const std::string& font_filename, unsigned int font_default_size, const int window_width_px, const int window_height_px) {
        this->font_default_size = float(font_default_size);
#pragma region Shader stuff...
        // const auto& reg = MapManager::get_instance().get_active_registry();
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(window_width_px), 0.0f, static_cast<float>(window_height_px));
        m_font_shader.init("font");
        m_font_shader.bind();
        m_font_shader.set_uniform_mat4f("projection", projection);
        // font buffer setup
        glGenVertexArrays(1, &m_font_vao);
        glGenBuffers(1, &m_font_vbo);
        /*
        // read in our shader files
        std::string vertexShaderSource = read_shader_file(PROJECT_SOURCE_DIR + std::string("shaders/font.vs.glsl"));
        std::string fragmentShaderSource = read_shader_file(PROJECT_SOURCE_DIR + std::string("shaders/font.fs.glsl"));
        const char* vertexShaderSource_c = vertexShaderSource.c_str();
        const char* fragmentShaderSource_c = fragmentShaderSource.c_str();

        // enable blending or you will just get solid boxes instead of text
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // font buffer setup
        glGenVertexArrays(1, &m_font_vao);
        glGenBuffers(1, &m_font_vbo);

        // font vertex shader
        unsigned int font_vertexShader;
        font_vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(font_vertexShader, 1, &vertexShaderSource_c, NULL);
        glCompileShader(font_vertexShader);
        int result;
        glGetShaderiv(font_vertexShader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            Log::log_error_and_terminate("ERROR::SHADER::VERTEX::COMPILATION_FAILED", __FILE__, __LINE__);
        }

        // font fragement shader
        unsigned int font_fragmentShader;
        font_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(font_fragmentShader, 1, &fragmentShaderSource_c, NULL);
        glCompileShader(font_fragmentShader);
        glGetShaderiv(font_fragmentShader, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            Log::log_error_and_terminate("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED", __FILE__, __LINE__);
        }

        // font shader program
        m_font_shader = glCreateProgram();
        glAttachShader(m_font_shader, font_vertexShader);
        glAttachShader(m_font_shader, font_fragmentShader);
        glLinkProgram(m_font_shader);
        glValidateProgram(m_font_shader);
        gl_has_errors();

        // apply orthographic projection matrix for font, i.e., screen space
        glUseProgram(m_font_shader);
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(window_width_px), 0.0f, static_cast<float>(window_height_px));
        GLint project_location = glGetUniformLocation(m_font_shader, "projection");
        assert(project_location > -1);
        std::cout << "project_location: " << project_location << std::endl;
        glUniformMatrix4fv(project_location, 1, GL_FALSE, glm::value_ptr(projection));

        // clean up shaders
        glDeleteShader(font_vertexShader);
        glDeleteShader(font_fragmentShader);
        */
#pragma endregion

        // init FreeType fonts
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            Log::log_error_and_terminate("ERROR::FREETYPE: Could not init FreeType Library", __FILE__, __LINE__);
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, font_filename.c_str(), 0, &face)) {
            Log::log_error_and_terminate("ERROR::FREETYPE: Failed to load font: " + font_filename, __FILE__, __LINE__);
            return false;
        }

        // extract a default size
        FT_Set_Pixel_Sizes(face, 0, font_default_size);

        // disable byte-alignment restriction in OpenGL
        GL_Call(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));

        // load each of the chars - note only first 128 ASCII chars
        for (unsigned char c = (unsigned char)0; c < (unsigned char)128; c++) {
            // load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }

            // generate texture
            unsigned int texture;
            GL_Call(glGenTextures(1, &texture));
            GL_Call(glBindTexture(GL_TEXTURE_2D, texture));

            GL_Call(glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            ));

            // set texture options
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x),
                (char)c
            };
            m_ftCharacters.insert(std::pair<char, Character>(c, character));
        }
        GL_Call(glBindTexture(GL_TEXTURE_2D, 0));

        // clean up
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        // bind buffers
        GL_Call(glBindVertexArray(m_font_vao));
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, m_font_vbo));
        GL_Call(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW));
        GL_Call(glEnableVertexAttribArray(0));
        GL_Call(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0));

        // release buffers
        GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GL_Call(glBindVertexArray(0));

        return true;
    }

    void render_text(std::string text, float x, float y, float scale, const glm::vec3& color) {

		// activate the shader program
        m_font_shader.bind();

		// get shader uniforms
        m_font_shader.set_uniform_3f("textColor", color);
        const glm::mat4 trans = glm::mat4(1.0f);
        m_font_shader.set_uniform_mat4f("transform", trans);

		// use program, load variables, bind to VAO, then iterate thru chars
		GL_Call(glBindVertexArray(m_font_vao));
		// iterate through all characters
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++) {
			Character ch = m_ftCharacters[*c];

			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;
			// update VBO for each character
			float vertices[6][4] = {
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};

			// render glyph texture over quad
			GL_Call(glBindTexture(GL_TEXTURE_2D, ch.TextureID));
			// std::cout << "binding texture: " << ch.character << " = " << ch.TextureID << std::endl;

			// update content of VBO memory
			GL_Call(glBindBuffer(GL_ARRAY_BUFFER, m_font_vbo));
			GL_Call(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));
			GL_Call(glBindBuffer(GL_ARRAY_BUFFER, 0));

			// render quad
			GL_Call(glDrawArrays(GL_TRIANGLES, 0, 6));

			// now advance cursors for next glyph (note that advance is number of 1/64 pixels)
			x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
		}
		GL_Call(glBindVertexArray(0));
		GL_Call(glBindTexture(GL_TEXTURE_2D, 0));
	}
};