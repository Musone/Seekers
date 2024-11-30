#pragma once

#include <renderer/Renderer.hpp>
#include <renderer/Texture2D.hpp>
#include <renderer/FontStuff.hpp>

#include <vector>
#include <memory>           // for std::unique_ptr
#include <functional>       // for std::function, std::bind
#include <string>

// GLFW includes (if you're using GLFW)
#include <GLFW/glfw3.h>

// UI Element Base Class
class UIElement {
protected:
    glm::vec2 position;    
    glm::vec2 size;        
    bool is_hovered;
    bool is_active;
    Mesh* m_square_mesh;

public:
    UIElement(Mesh* square_mesh, const glm::vec2& pos, const glm::vec2& sz) 
        : m_square_mesh(square_mesh), position(pos), size(sz), is_hovered(false), is_active(false) {}

    virtual ~UIElement() {} // Virtual destructor for proper cleanup

    virtual void update(float mouse_x, float mouse_y) {
        is_hovered = (mouse_x >= position.x - size.x / 2.0f && mouse_x <= position.x + size.x / 2.0f &&
                     mouse_y >= position.y - size.y / 2.0f && mouse_y <= position.y + size.y / 2.0f);
    }

    virtual void draw(Renderer* renderer, Shader* shader) = 0;
    virtual void on_click() = 0;

    bool is_hovering() const { return is_hovered; }
};

// Button Class
class Button : public UIElement {
private:
    std::string text;
    std::function<void()> callback;
    Texture2D* normal_texture;
    Texture2D* hover_texture;

public:
    Button(Mesh* square_mesh, const glm::vec2& pos, const glm::vec2& sz, 
           const std::string& btn_text, 
           Texture2D* normal, Texture2D* hover,
           const std::function<void()>& on_click)  // Changed to const reference
        : UIElement(square_mesh, pos, sz)
        , text(btn_text)
        , normal_texture(normal)
        , hover_texture(hover)
        , callback(on_click) {}

    void draw(Renderer* renderer, Shader* shader) override {
        const float aspect_ratio = float(renderer->get_window_width()) / float(renderer->get_window_height());
        float window_width = float(renderer->get_window_width());
        float window_height = float(renderer->get_window_height());

        shader->set_uniform_3f("u_colour", is_hovered ? glm::vec3(0.9f) : glm::vec3(1.0f));
        shader->set_uniform_1i("u_texture", (is_hovered ? hover_texture : normal_texture)->bind(21));
        shader->set_uniform_mat4f("u_model",
            Transform::create_model_matrix(
                {position.x, position.y, 0.0f},
                {0, 0, 0},
                {size.x, size.y, 1}
            )
        );

        renderer->draw(*m_square_mesh, *shader);

        glm::vec2 text_pos = {
            (position.x / 2.0f + 0.5f) * float(renderer->get_window_width()),
            (position.y / 2.0f + 0.5f) * float(renderer->get_window_height())
        };
        glm::vec2 text_size = {
            size.x * float(renderer->get_window_width()),
            size.y * float(renderer->get_window_height())
        };
        
        auto& font_stuff= FontStuff::get_instance();
        font_stuff.render_text(
            text,
            text_pos.x - text.length()*8,
            text_pos.y - 12,
            1.0f,
            {1, 1, 1}
        );
    }

    void on_click() override {
        if (callback) callback();
    }
};

// Menu Class
class Menu {
private:
    typedef std::unique_ptr<UIElement> UIElementPtr;
    std::vector<UIElementPtr> elements;
    std::unique_ptr<Texture2D> background;

public:
    Menu(const std::string& background_file_name) {
        background = std::make_unique<Texture2D>(background_file_name);
    }

    void add_element(UIElementPtr element) {
        elements.push_back(std::move(element));
    }

    void update(float mouse_x, float mouse_y) {
        for (auto& element : elements) {
            element->update(mouse_x, mouse_y);
        }
    }

    void draw(Renderer* renderer, Shader* shader, Mesh& square_mesh) {
        renderer->begin_draw();
        renderer->disable_depth_test();
        const float aspect_ratio = float(renderer->get_window_width()) / float(renderer->get_window_height());
        shader->set_uniform_3f("u_colour", glm::vec3(1.0f));
        shader->set_uniform_1i("u_texture", background->bind(21));
        shader->set_uniform_1f("u_health_percentage", 1);
        shader->set_uniform_mat4f("u_model",
            Transform::create_model_matrix(
                {0.0f, 0.0f, 0.0f},
                {0, 0, 0},
                {2, 2, 1}
            )
        );
        renderer->draw(square_mesh, *shader);

        for (auto& element : elements) {
            element->draw(renderer, shader);
        }

        renderer->enable_depth_test();
        renderer->end_draw();
    }

    void handle_click(float mouse_x, float mouse_y) {
        for (auto& element : elements) {
            if (element->is_hovering()) {
                element->on_click();
                break;
            }
        }
    }
};