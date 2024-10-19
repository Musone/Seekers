#pragma once

#include <utils/Log.hpp>
#include <renderer/Texture2D.hpp>
#include <renderer/SkyboxTexture.hpp>
#include <unordered_map>

struct TextureInfo {
    unsigned int texture_slot_id;
    unsigned int width;
    unsigned int height;
};

class TextureMaster {
    unsigned int m_count;
    std::unordered_map<std::string, TextureInfo> m_cache;
    std::vector<Texture2D*> m_ptr_textures;
    TextureInfo m_skybox_info;
    SkyboxTexture* m_ptr_skybox_texture;
    TextureMaster() : m_count(0), m_ptr_skybox_texture(nullptr) {}
public:
    static TextureMaster& get_instance() {
		static TextureMaster instance;
		return instance;
	}

    ~TextureMaster() {
        Log::log_warning(
            "Texture Master destructor doesn't destroy Texture objects. This would cause a memory leak.", 
            __FILE__, 
            __LINE__
        );
        // for (Texture* tex: m_ptr_textures) {
        //     delete tex;
        // }
        // throw new "SORRY BUT THE TEXTURE MASTER DESTRUCTOR NOT IMPLEMENTED";
    }

    TextureInfo get_texture_info(const std::string& texture_name) {
        if (m_cache.find(texture_name) != m_cache.end()) {
            return m_cache[texture_name];
        }
        m_ptr_textures.push_back(new Texture2D(texture_name));
        Texture2D* texture = m_ptr_textures[m_count];
        const unsigned int texture_slot = ++m_count;
        texture->bind(texture_slot);
        m_cache[texture_name] = { texture_slot, texture->get_width(), texture->get_height() };
        return m_cache[texture_name];
    }

    TextureInfo load_skybox(const std::string& file_name) {
        if (m_ptr_skybox_texture != nullptr) {
            delete m_ptr_skybox_texture;
        }
        m_ptr_skybox_texture = new SkyboxTexture(file_name);
        Log::log_warning("Skybox is hardcoded to bind to texture slot 31. This could cause problems down the line.", __FILE__, __LINE__);
        m_ptr_skybox_texture->bind(31);
        m_skybox_info = { 31, m_ptr_skybox_texture->get_width(), m_ptr_skybox_texture->get_height() };
        return m_skybox_info;
    }

    TextureInfo get_skybox() {
        if (m_ptr_skybox_texture == nullptr) {
            Log::log_error_and_terminate(
                "Skybox is not loaded...", 
                __FILE__, 
                __LINE__
            );
        }
        return m_skybox_info;
    }

    Texture2D* get_texture(const unsigned int texture_slot_id) {
        Log::log_warning(
            "I am using the texture slot id to retrieve Texture objects. This will cause bugs once we start unbinding textures from vram.",
            __FILE__,
            __LINE__
        );
        return m_ptr_textures[texture_slot_id - 1];
    }
};