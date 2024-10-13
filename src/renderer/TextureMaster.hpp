#pragma once

#include <utils/Log.hpp>
#include <renderer/Texture.hpp>
#include <unordered_map>

struct TextureInfo {
    unsigned int texture_slot_id;
    unsigned int width;
    unsigned int height;
};

class TextureMaster {
    unsigned int m_count;
    std::unordered_map<std::string, TextureInfo> m_cache;
public:
    TextureMaster() : m_count(0) {}

    ~TextureMaster() {
        // throw new "SORRY BUT THE TEXTURE MASTER DESTRUCTOR NOT IMPLEMENTED";
    }

    TextureInfo get_texture(const std::string& texture_name) {
        if (m_cache.find(texture_name) != m_cache.end()) {
            return m_cache[texture_name];
        }
        Texture texture(texture_name);
        const unsigned int texture_slot = ++m_count;
        texture.bind(texture_slot);
        m_cache[texture_name] = { texture.get_width(), texture.get_height(), texture_slot };
        return m_cache[texture_name];
    }
};