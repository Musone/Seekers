#pragma once

#include <renderer/GLUtils.hpp>
#include <utils/FileSystem.hpp>
#include <utils/Log.hpp>

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>
#include <gl3w.h>

#define SHADER_PATH "shaders/"
#define VERTEX_SHADER_EXTENSION ".vs.glsl"
#define FRAGMENT_SHADER_EXTENSION ".fs.glsl"

struct ShaderSource {
    std::string vertex_shader_source;
    std::string fragment_shader_source;
};

class Shader {
    unsigned int m_id;
    std::string m_file_path;
    std::string m_vs_path;
    std::string m_fs_path;
    ShaderSource m_shader_source;
    bool m_is_initialized;
    std::unordered_map<std::string, int> m_location_cache;
public:
    Shader(): m_is_initialized(false) {}

    Shader(const std::string& file_name): m_is_initialized(false) {
        init(file_name);
    }
    
    ~Shader() {
        if (m_is_initialized) {
            GL_Call(glDeleteProgram(m_id));
        }
    }
    
    void init(const std::string& file_name) {
        m_file_path = SHADER_PATH + file_name;
        m_vs_path = m_file_path + VERTEX_SHADER_EXTENSION;
        m_fs_path = m_file_path + FRAGMENT_SHADER_EXTENSION;
        
        // this shader makes the triangle red.
        m_shader_source.vertex_shader_source = FileSystem::read_file(m_vs_path);
        m_shader_source.fragment_shader_source = FileSystem::read_file(m_fs_path);
        
        m_id = _create_shader(
            m_shader_source.vertex_shader_source, 
            m_shader_source.fragment_shader_source
        );

        m_is_initialized = true;
        Log::log_success("loaded " + m_vs_path + " & " + m_fs_path, __FILE__, __LINE__);
    }

    const void bind() const {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Shader not initialized", __FILE__, __LINE__);
        }
        GL_Call(glUseProgram(m_id));
    }

    const void unbind() const {
        GL_Call(glUseProgram(0));
    }

    void set_uniform_1i(const std::string& name, const int& value) {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Shader not initialized", __FILE__, __LINE__);
        }
        bind();
        GL_Call(glUniform1i(_get_uniform_location(name), value));
    }

    void set_uniform_1f(const std::string& name, const float& value) {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Shader not initialized", __FILE__, __LINE__);
        }
        bind();
        GL_Call(glUniform1f(_get_uniform_location(name), value));
    }

    void set_uniform_mat4f(const std::string& name, const glm::mat4& matrix) {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Shader not initialized", __FILE__, __LINE__);
        }
        bind();
        GL_Call(glUniformMatrix4fv(_get_uniform_location(name), 1, GL_FALSE, &matrix[0][0]));
    }

    void set_uniform_4f(const std::string& name, const glm::vec4& vector) {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Shader not initialized", __FILE__, __LINE__);
        }
        bind();
        GL_Call(glUniform4f(_get_uniform_location(name), vector.x, vector.y, vector.z, vector.w));
    }

    void set_uniform_3f(const std::string& name, const glm::vec3& vector) {
        if (!m_is_initialized) {
            Log::log_error_and_terminate("Shader not initialized", __FILE__, __LINE__);
        }
        bind();
        GL_Call(glUniform3f(_get_uniform_location(name), vector.x, vector.y, vector.z));
    }

private:
    // The Cherno goes over initializing shaders in this video. Check it out if you are curious.
    // https://www.youtube.com/watch?v=71BLZwRGUJE&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=7&ab_channel=TheCherno
    unsigned int _compile_shader(const int& type, const std::string& source) {
        GL_Call(unsigned int id = glCreateShader(type));
        const char* src = source.c_str();
        GL_Call(glShaderSource(id, 1, &src, nullptr));
        GL_Call(glCompileShader(id));

        int result;
        GL_Call(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

        if (result == GL_FALSE) {
            int length;
            GL_Call(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
            char* error_message = (char*)alloca(length * sizeof(char));
            GL_Call(glGetShaderInfoLog(id, length, &length, error_message));
            std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
            std::cout << error_message << std::endl;
            GL_Call(glDeleteShader(id));
            return 0;
        }

        return id;
    }

    // The Cherno goes over initializing shaders in this video. Check it out if you are curious.
    // https://www.youtube.com/watch?v=71BLZwRGUJE&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=7&ab_channel=TheCherno
    unsigned int _create_shader(const std::string& vertex_shader, const std::string& fragment_shader) {
        GL_Call(unsigned int program = glCreateProgram());
        unsigned int compiled_vertex_shader = _compile_shader(GL_VERTEX_SHADER, vertex_shader);
        unsigned int compiled_fragment_shader = _compile_shader(GL_FRAGMENT_SHADER, fragment_shader);

        GL_Call(glAttachShader(program, compiled_vertex_shader));
        GL_Call(glAttachShader(program, compiled_fragment_shader));
        GL_Call(glLinkProgram(program));
        GL_Call(glValidateProgram(program));

        GL_Call(glDeleteShader(compiled_vertex_shader));
        GL_Call(glDeleteShader(compiled_fragment_shader));
        
        return program;
    }

    unsigned int _get_uniform_location(const std::string& name) {
        if (m_location_cache.find(name) != m_location_cache.end()) {
            return m_location_cache[name];
        }

        GL_Call(int location = glGetUniformLocation(m_id, name.c_str()));
        if (location == -1) {
            // Log::log_warning("Uniform '" + name + "' does not exist or is never used in " + m_vs_path + " nor " + m_fs_path, __FILE__, __LINE__);
        } else {
            m_location_cache[name] = location;
        }

        return location;
    }
};