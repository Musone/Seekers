#pragma once

#include <utils/Log.hpp>

#include <vector>

#include <gl3w.h>

struct LayoutElement {
    unsigned int type;
    unsigned int count;
    unsigned int is_normalized;

    static unsigned int get_size_of_type(unsigned int type) {
        switch (type) {
            case GL_FLOAT: return 4;
            case GL_UNSIGNED_INT: return 4;
            case GL_UNSIGNED_BYTE: return 1;
            default:
                Log::log_error_and_terminate("Type not supported", __FILE__, __LINE__);
        }
        return 0;
    }
};

class VertexBufferLayout {
    std::vector<LayoutElement> m_elements;
    unsigned int m_stride;
public:
    VertexBufferLayout() : m_stride(0) {}
    ~VertexBufferLayout() {}
    
    template<typename T>
    void push(unsigned int count) {
        Log::log_error_and_terminate(
            "Element type is not supported. Consider using one of the types below.",
            __FILE__,
            __LINE__
        );
    }

    template<>
    void push<float>(unsigned int count) {
        m_elements.push_back({ GL_FLOAT, count, false });
        m_stride += count * sizeof(float);
    }

    template<>
    void push<unsigned int>(unsigned int count) {
        m_elements.push_back({ GL_UNSIGNED_INT, count, false });
        m_stride += count * sizeof(unsigned int);
    }

    template<>
    void push<unsigned char>(unsigned int count) {
        m_elements.push_back({ GL_UNSIGNED_BYTE, count, true });
        m_stride += count * sizeof(unsigned char);
    }

    inline const std::vector<LayoutElement>& get_elements() const { return m_elements; }

    inline const unsigned int get_stride() const { return m_stride; }
};