#pragma once

#include <utils/Transform.hpp>
#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>
#include <utils/Timer.h>
#include <renderer/Renderer.hpp>
#include <renderer/Camera.hpp>
#include <ecs/Registry.hpp>
#include <app/World.h>
#include <app/InputManager.hpp>
#include <app/TextureMaster.hpp>
#include <components/RenderComponents.hpp>

#include <globals/Globals.h>

class Application {
    
public:
    void run_game_loop() {
        Renderer& renderer = Renderer::get_instance();
        // TODO: the game obviously...

        while (!renderer.is_terminated()) {
            throw std::runtime_error("not implemented lol");
        };
    };

    void run_demo_world() {
        Renderer& renderer = Renderer::get_instance();
        // The renderer must be initialized before anything else.
        renderer.init(
            "World Demo",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            true,
            false
        );

        renderer.set_on_key_callback_fn((void*)InputManager::on_key_pressed);
        renderer.set_on_mouse_move_callback_fn((void*)InputManager::on_mouse_move);
        renderer.set_on_mouse_click_callback_fn((void*)InputManager::on_mouse_button_pressed);

        // World map setup.
        float vertices[] = {
            -0.5, -0.5, 0,   0, 0, // 0
             0.5, -0.5, 0,   1, 0, // 1
             0.5,  0.5, 0,   1, 1, // 2
            -0.5,  0.5, 0,   0, 1, // 3
        };
        unsigned int indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        IndexBuffer ibo(indices, Common::c_arr_count(indices));
        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(2); // uv
        VertexArray vao;
        vao.init();
        VertexBuffer vbo(vertices, sizeof(vertices));
        vao.add_buffer(vbo, layout);

        TextureMaster& master = TextureMaster::get_instance();
        TextureInfo map_texture_info = master.get_texture("disnie_map.jpg");
        Shader shader("MapDemo");

        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        cam.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });

        World world;
        world.demo_init();
        Registry& reg = Registry::get_instance();
        
        Timer timer;
        float time_of_last_frame = float(timer.GetTime());

        Shader health_shader("MapDemoHealth");

        while (!renderer.is_terminated()) {
            float delta_time = 0.001f * float(timer.GetTime()) - time_of_last_frame;
            while (delta_time < 1000.0f / 60.0f) { delta_time = 0.001f * (float(timer.GetTime()) - time_of_last_frame); }
            // std::cout << 1000.0f / delta_time  << '\n';
            world.step(delta_time);
            const Motion& player_motion = reg.motions.get(reg.player);
            
            float the_3d_angle = 0;
            glm::vec2 cam_dir;
            if (Globals::is_3d_mode) {
                // Press Z to toggle 3d mode
                cam.set_rotation({ 2 * PI / 6, 0, player_motion.angle });
                const auto temp = cam.rotate_to_camera_direction({ 0, 0, -1 });
                cam_dir = { temp.x, temp.y };
                cam_dir = Common::normalize(cam_dir);
                cam.set_position(glm::vec3(player_motion.position - (cam_dir * 5.0f), 7));
                the_3d_angle = PI / 2;
            } else {
                cam.set_position(glm::vec3(player_motion.position, CAMERA_DISTANCE_FROM_WORLD));
                cam.set_rotation({ 0, 0, player_motion.angle });
            }

            // _handle_free_camera_inputs(renderer, cam);
            renderer.begin_draw();

            // Render world
            shader.set_uniform_mat4f(
                "u_mvp", 
                cam.get_view_project_matrix()
                * Transform::create_scaling_matrix({ MAP_WIDTH, MAP_HEIGHT, 1 })
            );
            shader.set_uniform_1i("u_texture", map_texture_info.texture_slot_id);
            renderer.draw(vao, ibo, shader);

            // Render entities
            {
                float i = 0;
                float or_something = 0.001;
                for (const auto& textured_entity : reg.textures.entities) {
                    if (!reg.motions.has(textured_entity)) {
                        continue;
                    }

                    const auto& motion = reg.motions.get(textured_entity);
                    glm::vec2 motion_pos = { motion.position.x, motion.position.y };
                    const auto& tex_name = reg.textures.get(textured_entity);
                    const TextureInfo texture_info = master.get_texture(tex_name.name);

                    float z_index = 0.1 + (i++) * or_something;
                    if (reg.weapon_stats.has(textured_entity)) {
                        z_index += 0.1;
                        // Gonna put the weapon above entities.
                        for (const auto& attacker_entity : reg.attackers.entities) {
                            if (reg.attackers.get(attacker_entity).weapon_id == textured_entity.get_id()) {
                                // Move the weapon to the attacker position in 3d mode... We should really be using
                                // child components and a Scene Graph for this...
                                if (!reg.motions.has(attacker_entity)) { 
                                    Log::log_warning(
                                        "Why doesn't attacker entity " + std::to_string(attacker_entity.get_id()) + " have a motion?",
                                        __FILE__, __LINE__
                                    );
                                    break; 
                                }
                                const auto& attacker_motion = reg.motions.get(attacker_entity);
                                motion_pos = attacker_motion.position;
                                const auto temp = Transform::create_rotation_matrix({ 0, 0, -PI / 2 }) * glm::vec4(cam_dir, 0, 1);
                                glm::vec2 shift_weapon_to_the_right = { temp.x, temp.y };
                                shift_weapon_to_the_right = Common::normalize(shift_weapon_to_the_right);
                                shift_weapon_to_the_right *= 1.0f; // move it by 1 world unit
                                motion_pos += shift_weapon_to_the_right;
                                break;
                            }
                        }
                    }
                    
                    shader.set_uniform_1i("u_texture", texture_info.texture_slot_id);
                    if (Globals::is_3d_mode) {
                    shader.set_uniform_mat4f(
                            "u_mvp", 
                            cam.get_view_project_matrix() * Transform::create_model_matrix(
                                glm::vec3(motion_pos.x - z_index * cam_dir.x, motion_pos.y - z_index * cam_dir.y, motion.scale.y / 2), 
                                { the_3d_angle, 0, motion.angle }, 
                                glm::vec3(motion.scale, 1.0)
                            )
                        );
                    } else {
                        shader.set_uniform_mat4f(
                            "u_mvp", 
                            cam.get_view_project_matrix() * Transform::create_model_matrix(
                                glm::vec3(motion.position, z_index), 
                                { 0, 0, motion.angle }, 
                                glm::vec3(motion.scale, 1.0)
                            )
                        );
                    }
                    renderer.draw(vao, ibo, shader);
                }
            }

            {
                for (const auto& loco_entity : reg.locomotion_stats.entities) {
                    // Render health bar
                    if (!reg.motions.has(loco_entity)) { continue; }
                    const auto& loco = reg.locomotion_stats.get(loco_entity);
                    const auto& loco_motion = reg.motions.get(loco_entity);
#define HEALTH_BAR_HEIGHT 0.5f
                    
                    if (loco.max_health == 0) {
                        Log::log_warning(
                            "Entity " 
                            + std::to_string(loco_entity.get_id()) 
                            + " Max health is 0. division by 0 error.", 
                            __FILE__, __LINE__
                        );
                        continue;
                    }
                    const float health_percentage = loco.health / loco.max_health;

                    // Red health bar layer
                    float z_index = 1.1;
                    if (Globals::is_3d_mode) {
                        glm::vec3 health_bar_pos;
                        glm::vec3 health_bar_angle;
                        if (loco_entity.get_id() == reg.player.get_id()) {
                            health_bar_pos = glm::vec3(cam.get_position());
                            const auto cam_dir_3d = glm::normalize(cam.rotate_to_camera_direction({ 0, 0, -1 }));
                            const auto& temp = Transform::create_rotation_matrix({ cam.get_rotation().x - PI / 2, cam.get_rotation().y, cam.get_rotation().z }) * glm::vec4(0, 0, -1, 1);
                            health_bar_pos += (2.5f * glm::vec3(temp.x, temp.y, temp.z)) + (3.0f * cam_dir_3d);
                        } else {
                            health_bar_pos = glm::vec3({ loco_motion.position.x, loco_motion.position.y, loco_motion.scale.y + HEALTH_BAR_HEIGHT / 2 + 0.5});
                        }
                            health_bar_angle = cam.get_rotation();

                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            cam.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                health_bar_pos,
                                health_bar_angle,
                                glm::vec3({ loco_motion.scale.x, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 1, 0, 0 });
                        renderer.draw(vao, ibo, health_shader);

                        // Green health bar layer
                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            cam.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                health_bar_pos - (0.001f * cam.rotate_to_camera_direction({ 0, 0, -1 })),
                                health_bar_angle,
                                glm::vec3({ loco_motion.scale.x * health_percentage, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 0, 1, 0 });
                        renderer.draw(vao, ibo, health_shader);
                    } else {
                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            cam.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                glm::vec3({ loco_motion.position.x, loco_motion.position.y - loco_motion.scale.y / 2 - HEALTH_BAR_HEIGHT / 2, z_index }),
                                glm::vec3({ 0, 0, 0 }),
                                glm::vec3({ loco_motion.scale.x, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 1, 0, 0 });
                        renderer.draw(vao, ibo, health_shader);

                        z_index += 0.001;
                        // Green health bar layer
                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            cam.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                glm::vec3({ loco_motion.position.x, loco_motion.position.y - loco_motion.scale.y / 2 - HEALTH_BAR_HEIGHT / 2, z_index }),
                                glm::vec3({ 0, 0, Globals::is_3d_mode ? player_motion.angle : 0 }),
                                glm::vec3({ loco_motion.scale.x * health_percentage, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 0, 1, 0});
                        renderer.draw(vao, ibo, health_shader);
                    }
                }
            }

            // Draw the aim trajectory for the player
            // I'm using the Health bar shader because well... I just wanted
            // to draw rectangles with static colours instead of textures...
            health_shader.set_uniform_3f("u_colour", { 1, 0, 0.5 }); // fuschia croshair (pink)
            glm::vec3 crosshair_pos = { player_motion.position.x, player_motion.position.y, player_motion.scale.y / 2.0f };
            const float crosshair_length = 1000;
            crosshair_pos += glm::vec3(cam_dir, 0) * (crosshair_length / 2.0f);
            health_shader.set_uniform_mat4f(
                "u_mvp",
                cam.get_view_project_matrix()
                * Transform::create_model_matrix(
                    crosshair_pos,
                    glm::vec3({ 0, 0, player_motion.angle }),
                    glm::vec3({ 0.2, crosshair_length, 1 })
                )
            );
            renderer.draw(vao, ibo, health_shader);

            renderer.end_draw();
            time_of_last_frame = float(timer.GetTime());
        }
    }

    void run_demo_camera() {
        Renderer& renderer = Renderer::get_instance();
        // The renderer must be initialized before anything else.
        renderer.init(
            "Camera Demo",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
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

        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        cam.set_rotation({ PI / 2, 0, -PI / 4 });

        while (!renderer.is_terminated()) {
            // Timer timer;
            renderer.begin_draw();

            _handle_free_camera_inputs(renderer, cam);

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
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
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
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
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
    void _handle_free_camera_inputs(const Renderer& renderer, Camera& cam) {
        glm::vec3 moveDirection(0.0f);
            glm::vec3 rotateDirection(0.0f);

            float moveSpeed = 0.05f;
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
    }
};
