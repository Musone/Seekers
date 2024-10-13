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
    std::vector<Texture*> m_ptr_textures;
    TextureMaster() : m_count(0) {}
public:
    static TextureMaster& get_instance() {
		static TextureMaster instance;
		return instance;
	}

    ~TextureMaster() {
        // throw new "SORRY BUT THE TEXTURE MASTER DESTRUCTOR NOT IMPLEMENTED";
    }

    TextureInfo get_texture(const std::string& texture_name) {
        if (m_cache.find(texture_name) != m_cache.end()) {
            return m_cache[texture_name];
        }
        // m_textures.emplace_back(texture_name);
        m_ptr_textures.push_back(new Texture(texture_name));
        const Texture* texture = m_ptr_textures[m_count];
        const unsigned int texture_slot = ++m_count;
        texture->bind(texture_slot);
        m_cache[texture_name] = { texture_slot, texture->get_width(), texture->get_height() };
        return m_cache[texture_name];
    }
};