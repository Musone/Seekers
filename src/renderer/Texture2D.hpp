#pragma once

#include <utils/Log.hpp>
#include <renderer/GLUtils.hpp>

#include <string>

#include <gl3w.h>
#include <glfw/glfw3.h>

// Documentation in ext/stb_image/stb_image.h says to do this.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// Don't touch it ;)

#define CHANNELS_RGBA 4
#define TEXTURE_PATH "textures/"

class Texture2D {
    unsigned int m_id;
    std::string m_file_path;
    unsigned char* m_local_buffer;
    int m_width;
    int m_height;
    int m_bits_per_pixel;
public:
    Texture2D(const std::string& name) : 
        m_id(0),
        m_file_path(TEXTURE_PATH + name), 
        m_local_buffer(nullptr),
        m_width(0), 
        m_height(0), 
        m_bits_per_pixel(0) {
            // OpenGL expects our texture pixels to start from the bottom left. However,
            // standard image formats like png and jpg start from the top left.
            stbi_set_flip_vertically_on_load(1);

            m_local_buffer = stbi_load(m_file_path.c_str(), &m_width, &m_height, &m_bits_per_pixel, CHANNELS_RGBA);

            GL_Call(glGenTextures(1, &m_id));
            GL_Call(glBindTexture(GL_TEXTURE_2D, m_id));

            // If you don't set all these texture parameters, you will get a black screen...
            // We love OpenGL!
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

            GL_Call(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_local_buffer));
            GL_Call(glBindTexture(GL_TEXTURE_2D, 0));
    }

    ~Texture2D() {
        stbi_image_free(m_local_buffer);
        GL_Call(glDeleteTextures(1, &m_id));
    }

    inline unsigned int get_width() const { return m_width; }
    inline unsigned int get_height() const { return m_height; }

    // GPU texture slots are limited. The graphics card has a fixed number of texture slots,
    // so if you need more than however many the graphics card supports, you will have to 
    // free up texture slots and manage them appropriately.
    //
    // OPENGL ONLY SUPPORTS 32 SLOTS TOTAL: This function will crash if not 0 < texture_slot < 32.
    // I am using texture slot 0 as a garbage bin to prevent texture creation order from causing
    // bugs (when using multiple textures).
    const void bind(unsigned int texture_slot) const {
        if (texture_slot > 31 || texture_slot < 1) {
            Log::log_error_and_terminate("'texture_slot' cannot exceed 31 or be less than 1", __FILE__, __LINE__);
        }
        GL_Call(glActiveTexture(GL_TEXTURE0 + texture_slot));
        GL_Call(glBindTexture(GL_TEXTURE_2D, m_id));
        GL_Call(glActiveTexture(GL_TEXTURE0));
    }

    const void unbind() const {
        GL_Call(glBindTexture(GL_TEXTURE_2D, 0));
    }

    const void enable_wrapping() const {
        GL_Call(glBindTexture(GL_TEXTURE_2D, m_id));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        GL_Call(glBindTexture(GL_TEXTURE_2D, 0));
    }

    const void disable_wrapping() const {
        GL_Call(glBindTexture(GL_TEXTURE_2D, m_id));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_Call(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_Call(glBindTexture(GL_TEXTURE_2D, 0));
    }
};