#pragma once

#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>
#include <ecs/Registry.hpp>
#include <renderer/Renderer.hpp>
#include <renderer/Camera.hpp>

class Application {
    
public:
    void run_game_loop() {
        Renderer& renderer = Renderer::get_instance();
        // TODO: the game obviously...

        while (!renderer.is_terminated()) {
            throw std::runtime_error("not implemented lol");
        };
    };

    void run_demo_camera() {
        Renderer& renderer = Renderer::get_instance();
        // The renderer must be initialized before anything else.
        renderer.init(
            "Camera Demo",
            1920,
            1280,
            true,
            false
        );
        
        float cube_vertices[] = {
            // Front face (Red)
            1, 1, 0,  1, 0, 0,
            2, 1, 0,  1, 0, 0,
            2, 2, 0,  1, 0, 0,
            1, 2, 0,  1, 0, 0,

            // Back face (Green)
            1, 1, 1,  0, 1, 0,
            2, 1, 1,  0, 1, 0,
            2, 2, 1,  0, 1, 0,
            1, 2, 1,  0, 1, 0,

            // Left face (Blue)
            1, 1, 0,  0, 0, 1,
            1, 2, 0,  0, 0, 1,
            1, 2, 1,  0, 0, 1,
            1, 1, 1,  0, 0, 1,

            // Right face (Yellow)
            2, 1, 0,  1, 1, 0,
            2, 2, 0,  1, 1, 0,
            2, 2, 1,  1, 1, 0,
            2, 1, 1,  1, 1, 0,

            // Top face (Magenta)
            1, 2, 0,  1, 0, 1,
            2, 2, 0,  1, 0, 1,
            2, 2, 1,  1, 0, 1,
            1, 2, 1,  1, 0, 1,

            // Bottom face (Cyan)
            1, 1, 0,  0, 1, 1,
            2, 1, 0,  0, 1, 1,
            2, 1, 1,  0, 1, 1,
            1, 1, 1,  0, 1, 1
        };

        unsigned int indices[] = {
            0,  1,  2,  2,  3,  0,  // Front face
            4,  5,  6,  6,  7,  4,  // Back face
            8,  9,  10, 10, 11, 8,  // Left face
            12, 13, 14, 14, 15, 12, // Right face
            16, 17, 18, 18, 19, 16, // Top face
            20, 21, 22, 22, 23, 20  // Bottom face
        };
        VertexBufferLayout layout;        
        layout.push<float>(3); // xyz
        layout.push<float>(3); // rgb

        IndexBuffer ibo;
        ibo.init(indices, Common::c_arr_count(indices));
        
        VertexArray cube_vao;
        VertexBuffer cube_vbo;
        cube_vao.init();
        cube_vbo.init(cube_vertices, sizeof(cube_vertices));
        cube_vao.add_buffer(cube_vbo, layout);

        Shader shader;
        shader.init("CameraDemo");

        Camera cam;
        cam.set_rotation({ PI / 2, 0, -PI / 4 });

        while (!renderer.is_terminated()) {
            // Timer timer;
            renderer.begin_draw();

#pragma region TEST INPUTS
            glm::vec3 moveDirection(0.0f);
            glm::vec3 rotateDirection(0.0f);

            float moveSpeed = 0.005f;
            float rotateSpeed = PI / 300;  // radians per frame

            glm::vec3 newPosition = 1.f * cam.get_position();
            glm::vec3 newRotation = 1.f * cam.get_rotation();
            bool pos_changed = false;
            bool rot_changed = false;

            glm::vec3 player_input(0.0f);

            // Handle keyboard input for movement
            if (renderer.is_key_pressed(GLFW_KEY_W)) {
                player_input.z -= moveSpeed;  // Move in negative Z
                pos_changed = true;
            }
            if (renderer.is_key_pressed(GLFW_KEY_S)) {
                player_input.z += moveSpeed;  // Move in positive Z
                pos_changed = true;
            }
            if (renderer.is_key_pressed(GLFW_KEY_A)) {
                player_input.x -= moveSpeed;  // Move in negative X
                pos_changed = true;
            }
            if (renderer.is_key_pressed(GLFW_KEY_D)) {
                player_input.x += moveSpeed;  // Move in positive X
                pos_changed = true;
            }
            if (renderer.is_key_pressed(GLFW_KEY_SPACE)) {
                player_input.y += moveSpeed;  // Move in positive Y
                pos_changed = true;
            }
            if (renderer.is_key_pressed(GLFW_KEY_LEFT_CONTROL)) {
                player_input.y -= moveSpeed;  // Move in negative Y
                pos_changed = true;
            }

            // Handle keyboard input for rotation
            if (renderer.is_key_pressed(GLFW_KEY_UP)) {
                    rot_changed = true;
                    newRotation.x += rotateSpeed;
            }
            if (renderer.is_key_pressed(GLFW_KEY_DOWN)) {
                    rot_changed = true;
                    newRotation.x -= rotateSpeed;
            }
            if (renderer.is_key_pressed(GLFW_KEY_LEFT)) {
                    rot_changed = true;
                    newRotation.z += rotateSpeed;
            }
            if (renderer.is_key_pressed(GLFW_KEY_RIGHT)) {
                    rot_changed = true;
                    newRotation.z -= rotateSpeed;
            }

            // Update camera position and rotation using the setter methods
            if (rot_changed) {
                cam.set_rotation(newRotation);
            }
            if (pos_changed) {
                newPosition += cam.rotate_to_camera_direction(player_input);
                cam.set_position(newPosition);
            }
#pragma endregion

            // Render the player.
            shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());
            renderer.draw(cube_vao, ibo, shader);

            renderer.end_draw();
        }
    }

    void run_demo_texture() {
        Renderer& renderer = Renderer::get_instance();
        // The renderer must be initialized before anything else.
        renderer.init(
            "Texture Demo",
            1920,
            1280,
            true,
            false
        );
        
        float player_vertices[] = {
            // We have to manually map textures to our vertices. in UV coordinates,
            // the bottom left of the texture is <0,0> and the top right is <1,1>.
            -0.5f, -0.5f, 0.0f, 0.0f, // bottom left
            0.0f, -0.5f, 1.0f, 0.0f, // bottom right
            0.0f, 0.5f, 1.0f, 1.0f, // top right
            -0.5f, 0.5f, 0.0f, 1.0f // top left
        };
        // I set it up so that player and skeleton can use the same indices.
        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        VertexBufferLayout layout;
        // 2 floats form a vec2 that we use for position.
        layout.push<float>(2);
        // 2 float form a vec2 that we use for UV coordinates.
        layout.push<float>(2);
        // The player and skeleton can use the same ibo because I made the indices match.
        IndexBuffer ibo;
        ibo.init(indices, Common::c_arr_count(indices));
        
        VertexArray player_vao;
        VertexBuffer player_vbo;
        player_vao.init();
        player_vbo.init(player_vertices, sizeof(player_vertices));
        player_vao.add_buffer(player_vbo, layout);
        Texture player_texture("player.png");
        const unsigned int player_texture_slot = 1;
        player_texture.bind(player_texture_slot);
        
        float skeleton_vertices[] = {
            0.0f, -0.5f, 0.0f, 0.0f, // bottom left
            0.5f, -0.5f, 1.0f, 0.0f, // bottom right
            0.5f, 0.5f, 1.0f, 1.0f, // top right
            0.0f, 0.5f, 0.0f, 1.0f // top left
        };
        VertexArray skeleton_vao;
        VertexBuffer skeleton_vbo;
        skeleton_vao.init();
        skeleton_vbo.init(skeleton_vertices, sizeof(skeleton_vertices));
        skeleton_vao.add_buffer(skeleton_vbo, layout);
        Texture skeleton_texture("skeleton.png");
        const unsigned int skeleton_texture_slot = 2;
        skeleton_texture.bind(skeleton_texture_slot);

        // src/shaders/TextureDemo.vs.glsl & src/shaders/TextureDemo.fs.glsl
        Shader shader;
        shader.init("TextureDemo");
        
        while (!renderer.is_terminated()) {
            renderer.begin_draw();

            // Render the player.
            shader.set_uniform_1i("u_texture", player_texture_slot);
            renderer.draw(player_vao, ibo, shader);

            // Then render the skeleton.
            shader.set_uniform_1i("u_texture", skeleton_texture_slot);
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
            shader.set_uniform_4f("u_coooooolor", blue);

            renderer.draw(vao, ibo, shader);

            renderer.end_draw();
        }
    }
#pragma endregion
private:
};