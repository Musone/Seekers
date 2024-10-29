#pragma once

#include <utils/Log.hpp>
#include <renderer/GLUtils.hpp>

#include <string>
#include <vector>

#include <gl3w.h>
#include <glfw/glfw3.h>

// Copy pasta from Texture2D with some modifications... This is a massive code smell but
// I don't have time to architect this nicely...

// Documentation in ext/stb_image/stb_image.h says to do this.
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>
// Don't touch it ;)

#define CHANNELS_RGB 3
#define TEXTURE_PATH "textures/"

class SkyboxTexture {
    unsigned int m_id;
    std::string m_file_path;
    unsigned char* m_local_buffer;
    int m_width;
    int m_height;
    int m_bits_per_pixel;
public:
    SkyboxTexture(const std::string& name) : 
        m_id(0),
        m_file_path(TEXTURE_PATH + name), 
        m_local_buffer(nullptr),
        m_width(0), 
        m_height(0), 
        m_bits_per_pixel(0) {
            stbi_set_flip_vertically_on_load(false);
            m_local_buffer = stbi_load(m_file_path.c_str(), &m_width, &m_height, &m_bits_per_pixel, CHANNELS_RGB);
            
            if (!m_local_buffer) {
                Log::log_error_and_terminate("Failed to load skybox texture: " + m_file_path, __FILE__, __LINE__);
            }

            GL_Call(glGenTextures(1, &m_id));
            GL_Call(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));

            int faceWidth = m_width / 4;
            int faceHeight = m_height / 3;

            // Define the order of faces based on your layout
            const int faceOrder[6][2] = {
                {2, 1}, // right
                {0, 1}, // left
                {1, 1}, // front
                {3, 1}, // back
                {1, 0}, // top
                {1, 2}  // bottom
            };

            for (unsigned int i = 0; i < 6; ++i) {
                int xOffset = faceOrder[i][0] * faceWidth;
                int yOffset = faceOrder[i][1] * faceHeight;

                std::vector<unsigned char> faceData(faceWidth * faceHeight * CHANNELS_RGB);
                for (int y = 0; y < faceHeight; ++y) {
                    for (int x = 0; x < faceWidth; ++x) {
                        int srcX, srcY;
                        
                        if (i == 0) { // Right face 
                            srcX = xOffset + y;
                            srcY = yOffset + x;
                        } else if (i == 1) { // Left
                            srcX = xOffset + (faceWidth - 1 - y);  
                            srcY = yOffset + (faceHeight - 1 - x); 
                        } else if (i == 3) { // Back
                            srcX = xOffset + (faceWidth - 1 - x);
                            srcY = yOffset + y;
                        } else if (i == 4) { // Top
                            srcX = xOffset + x;
                            srcY = yOffset + (faceHeight - 1 - y);
                        } else if (i == 5) { // Bottom
                            srcX = xOffset + (faceWidth - 1 - x);
                            srcY = yOffset + y;
                        } else { // Front 
                            srcX = xOffset + x;
                            srcY = yOffset + (faceHeight - 1 - y);
                        }
                        
                        // Ensure we're not going out of bounds
                        if (srcX >= m_width || srcY >= m_height) {
                            continue;  // Skip this pixel if it's out of bounds
                        }

                        for (int c = 0; c < CHANNELS_RGB; ++c) {
                            int srcIndex = (srcY * m_width + srcX) * CHANNELS_RGB + c;
                            int destIndex = (y * faceWidth + x) * CHANNELS_RGB + c;
                            faceData[destIndex] = m_local_buffer[srcIndex];
                        }
                    }
                }

                GL_Call(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                                    0, GL_RGB, faceWidth, faceHeight, 0, 
                                    GL_RGB, GL_UNSIGNED_BYTE, faceData.data()));
            }

            GL_Call(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            GL_Call(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            GL_Call(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GL_Call(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GL_Call(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

            GL_Call(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    }

    ~SkyboxTexture() {
        stbi_image_free(m_local_buffer);
        GL_Call(glDeleteTextures(1, &m_id));
    }

    inline unsigned int get_width() const { return m_width; }
    inline unsigned int get_height() const { return m_height; }

    unsigned int bind(unsigned int texture_slot) const {
        if (texture_slot > 31 || texture_slot < 1) {
            Log::log_error_and_terminate("'texture_slot' cannot exceed 31 or be less than 1", __FILE__, __LINE__);
        }
        GL_Call(glActiveTexture(GL_TEXTURE0 + texture_slot));
        GL_Call(glBindTexture(GL_TEXTURE_CUBE_MAP, m_id));
        GL_Call(glActiveTexture(GL_TEXTURE0));
        return texture_slot;
    }

    const void unbind() const {
        GL_Call(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
    }
};