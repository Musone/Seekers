#pragma once

#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>
#include <renderer/Renderer.hpp>
#include <ecs/Registry.hpp>

class Application {
    

public:
    void run_game_loop() {
        Renderer& renderer = Renderer::get_instance();
        /* Loop until the user closes the window */
        // shader.set_uniform_4f("u_coooooolor", { 0, 0, 1, 1 });
        while (!renderer.is_terminated()) {
            // renderer.draw();
        };
    };

    void run_test_loop() {
        Vertex vertices[] = {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // 0
            {{0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}}, // 1
            {{0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}, // 2
            {{-0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}}, // 3
        };

        Renderer& renderer = Renderer::get_instance();
        VertexArray vao;
        IndexBuffer ibo;
        VertexBuffer vbo;

        // The renderer must be initialized before anything else.
        renderer.init(
            "Hello World",
            1920,
            1280,
            true,
            false
        );
        vao.init();
        vbo.init(vertices, sizeof(vertices));
        
        VertexBufferLayout layout;
        layout.push<float>(2);
        layout.push<float>(3);

        // Link the Attribute layout with the Vertex Buffer.
        vao.add_buffer(vbo, layout);

        // Index buffer lets us reuse vertices to reduce vram consumption.
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 1
        };
        ibo.init(indices, Common::c_arr_count(indices));

        // src/shaders/hello.vs.glsl & src/shaders/hello.fs.glsl
        Shader shader;
        shader.init("Hello");

        glm::vec4 blue(0, 0, 1, 1);

        while (!renderer.is_terminated()) {
            renderer.clear();

            // The name of the uniform has to match exactly with the name in the shader.
            // The shader needs to be bound before you can set its' uniform.
            //
            // src/shaders/hello.fs.glsl:
            //      ...
            //      uniform vec4 u_coooooolor;
            //      ...
            shader.bind();
            shader.set_uniform_4f("u_coooooolor", blue);

            renderer.draw(vao, ibo, shader);
        }
    }

private:
    void _test_demo_setup() {
        
    }
};