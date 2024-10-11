#pragma once

#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>
#include <ecs/Registry.hpp>
#include <renderer/Renderer.hpp>

class Application {
    
public:
    void run_game_loop() {
        Renderer& renderer = Renderer::get_instance();
        // TODO: the game obviously...

        while (!renderer.is_terminated()) {
            throw std::runtime_error("not implemented lol");
        };
    };

    void run_demo_texture() {
        float player_vertices[] = {
            // We have to manually map textures to our vertices. in UV coordinates,
            // the bottom left of the texture is <0,0> and the top right is <1,1>.
            -0.5f, -0.5f, 0.0f, 0.0f, // bottom left
            0.0f, -0.5f, 1.0f, 0.0f, // bottom right
            0.0f, 0.5f, 1.0f, 1.0f, // top right
            -0.5f, 0.5f, 0.0f, 1.0f // top left
        };
        float skeleton_vertices[] = {
            0.0f, -0.5f, 0.0f, 0.0f, // bottom left
            0.5f, -0.5f, 1.0f, 0.0f, // bottom right
            0.5f, 0.5f, 1.0f, 1.0f, // top right
            0.0f, 0.5f, 0.0f, 1.0f // top left
        };
        // I set it up so that player and skeleton can use the same indices.
        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        Renderer& renderer = Renderer::get_instance();
        // The renderer must be initialized before anything else.
        renderer.init(
            "Texture Demo",
            1920,
            1280,
            true,
            false
        );
        VertexArray player_vao;
        VertexArray skeleton_vao;
        player_vao.init();
        skeleton_vao.init();
        
        IndexBuffer ibo;
        ibo.init(indices, Common::c_arr_count(indices));
        
        VertexBuffer player_vbo;
        VertexBuffer skeleton_vbo;

        player_vbo.init(player_vertices, sizeof(player_vertices));
        skeleton_vbo.init(skeleton_vertices, sizeof(skeleton_vertices));

        VertexBufferLayout layout;
        // 2 floats form a vec2 that we use for position.
        layout.push<float>(2);
        // 2 float form a vec2 that we use for UV coordinates.
        layout.push<float>(2);
        
        player_vao.add_buffer(player_vbo, layout);
        skeleton_vao.add_buffer(skeleton_vbo, layout);

        // src/shaders/TextureDemo.vs.glsl & src/shaders/TextureDemo.fs.glsl
        Shader shader;
        shader.init("TextureDemo");

        Texture player_texture("player.png");
        Texture skeleton_texture("skeleton.png");
        player_texture.bind(0);
        skeleton_texture.bind(1);
        
        while (!renderer.is_terminated()) {
            renderer.begin_draw();

            ibo.bind();
            shader.bind();

            // Render the player.
            player_vao.bind();
            shader.set_uniform_1i("u_texture", 0);
            renderer.draw(player_vao, ibo, shader);

            // Then render the skeleton.
            skeleton_vao.bind();
            shader.set_uniform_1i("u_texture", 1);
            renderer.draw(skeleton_vao, ibo, shader);

            renderer.end_draw();
        }
    }

    void run_demo_basic() {
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
            renderer.begin_draw();

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

            renderer.end_draw();
        }
    }

private:
};