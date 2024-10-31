#pragma once

#include <utils/Transform.hpp>
#include <utils/Log.hpp>
#include <utils/FileSystem.hpp>
#include <utils/Timer.h>
#include <utils/Common.hpp>
#include <renderer/Renderer.hpp>
#include <renderer/Camera.hpp>
#include <renderer/SkyboxTexture.hpp>
#include <renderer/Mesh.hpp>
#include <renderer/ModelBase.hpp>
#include <renderer/AnimatedModel.hpp>
#include <renderer/StaticModel.hpp>
#include <ecs/Registry.hpp>
#include <app/World.h>
#include <app/InputManager.hpp>
#include <app/TextureMaster.hpp>
#include <components/RenderComponents.hpp>
#include <components/CombatComponents.hpp>
#include <components/EntityIdentifierComponents.hpp>

#include <globals/Globals.h>

#include <utils/CalladaTokenizer.hpp>

#include <iomanip>
#include <vector>
#include <unordered_map>

class Application {
    Renderer* m_renderer = nullptr;
    Camera m_camera;

    Mesh m_square_mesh;
    Mesh m_cube_mesh;
    
    SkyboxTexture* m_skybox_texture;
    Shader* m_skybox_shader;
    
    Texture2D* m_wall_texture;
    Shader* m_wall_shader;

    StaticModel* m_spooky_tree;

    Texture2D* m_map_texture;
    Shader* m_floor_shader;

    glm::vec3 m_light_pos;
    glm::vec3 m_light_colour;

    std::unordered_map<unsigned int, AnimatedModel*> m_models;
public:

    Application() : m_light_pos(1.0f, 1.0f, 2.0f) {
        // Setup
        m_renderer = &Renderer::get_instance();
        m_renderer->init(
            "Seekers",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            true,
            false
        );
        m_camera.init(m_renderer->get_window_width(), m_renderer->get_window_height());

        m_skybox_shader = new Shader("Skybox");
        m_skybox_texture = new SkyboxTexture("random_skybox.png");
        m_map_texture = new Texture2D("jungle_tile_1.jpg");
        m_wall_texture = new Texture2D("tileset_1.png");
        m_wall_shader = new Shader("StaticBlinnPhong");
        m_floor_shader = new Shader("StaticBlinnPhong");

        VertexBufferLayout cube_layout;
        cube_layout.push<float>(3); // position
        cube_layout.push<float>(3); // normal
        cube_layout.push<float>(4); // vertex color (RGBA)
        cube_layout.push<float>(2); // uv
        m_cube_mesh.init(
            m_cube_vertices.data(), 
            m_cube_indices.data(), 
            sizeof(m_cube_vertices[0]) * m_cube_vertices.size(), 
            m_cube_indices.size(), 
            cube_layout
        );


        VertexBufferLayout square_layout;
        square_layout.push<float>(3); // position
        square_layout.push<float>(3); // normal
        square_layout.push<float>(4); // vertex color (RGBA)
        square_layout.push<float>(2); // uv
        m_square_mesh.init(
            m_square_vertices.data(), 
            m_square_indices.data(), 
            sizeof(m_square_vertices[0]) * m_square_vertices.size(), 
            m_square_indices.size(), 
            square_layout
        );
        m_skybox_shader->set_uniform_1i("u_skybox", m_skybox_texture->bind(31));

        m_spooky_tree = new StaticModel("models/Spooky Tree/Spooky Tree.obj", m_wall_shader);
        m_spooky_tree->m_has_texture = true;
        m_spooky_tree->texture_list.push_back(std::make_shared<Texture2D>("Spooky Tree.jpg"));
        m_spooky_tree->mesh_list.back()->set_texture(m_spooky_tree->texture_list.back());

        m_light_colour = glm::vec3(1.0f);
    }

    ~Application() {
        delete m_skybox_shader;
        delete m_skybox_texture;
        delete m_map_texture;
        delete m_floor_shader;
    }

    void run_game_loop() { 
        m_camera.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });

        // Get keys inputs from input manager
        m_renderer->set_on_key_callback_fn((void*)InputManager::on_key_pressed);
        m_renderer->set_on_mouse_move_callback_fn((void*)InputManager::on_mouse_move);
        m_renderer->set_on_mouse_click_callback_fn((void*)InputManager::on_mouse_button_pressed);

        Shader animated_shader("AnimatedBlinnPhong");
        Shader static_shader("StaticBlinnPhong");
        
        AnimatedModel hero("models/Hero/Hero.dae", &animated_shader);
        hero.load_animation_from_file("models/Hero/Left.dae");
        hero.load_animation_from_file("models/Hero/Right.dae");
        hero.load_animation_from_file("models/Hero/Backward.dae");
        hero.load_animation_from_file("models/Hero/Forward.dae");
        hero.load_animation_from_file("models/Hero/Roll.dae");
        hero.load_animation_from_file("models/Hero/Standing Attack.dae");
        hero.load_animation_from_file("models/Hero/Running Attack.dae");
        hero.set_rotation_x(PI / 2);
        hero.set_scale(glm::vec3(0.02));
        hero.print_bones();
        hero.print_animations();

        AnimatedModel warrior_grunt("models/Warrior Grunt/Warrior Grunt (drake).dae", &animated_shader);
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Left.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Right.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Backward.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Forward.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Roll.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Standing Attack.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Running Attack.dae");
        warrior_grunt.set_rotation_x(PI / 2);
        warrior_grunt.set_scale(glm::vec3(0.02));
        warrior_grunt.print_bones();
        warrior_grunt.print_animations();

        // Texture2D old_hero("player.png");

        if (hero.get_animation_count() > 0) {
            hero.play_animation(0);
        }

        World world;
        world.demo_init();
        Registry& reg = Registry::get_instance();

        unsigned int counter = 1;
        for (const auto& entity : reg.motions.entities) {
            if (entity.get_id() == reg.player.get_id()) { continue; }
            if (reg.locomotion_stats.has(entity)) {
                const auto& motion = reg.motions.get(entity);
                m_models[entity.get_id()] = new AnimatedModel(warrior_grunt, counter++);
            }
        }

        Timer timer;
        float time_of_last_frame = 0;
        const float FRAME_TIME_60FPS = 1000000.0f / 60.0f;
        float base_camera_speed = 1.0f;
        float camera_speed = base_camera_speed;
        while (!m_renderer->is_terminated()) {
            float delta_time = 0.001f * float(timer.GetTime()) - time_of_last_frame;
            while (delta_time < 1000.0f / 60.0f) { delta_time = 0.001f * (float(timer.GetTime()) - time_of_last_frame); }
            // float delta_time = float(timer.GetTime()) - time_of_last_frame;
            // while (delta_time < FRAME_TIME_60FPS) {
            //     delta_time = float(timer.GetTime()) - time_of_last_frame;
            // }
            
            if (Globals::is_3d_mode) {
                // m_renderer->lock_cursor();
            } else {
                m_renderer->terminate();
            }

            world.step(delta_time);
            const Motion& player_motion = reg.motions.get(reg.player);
            glm::vec2 cam_dir;
            glm::vec3 ortho_cam_dir;
            {
                float the_3d_angle = 0;
                // m_camera.set_rotation({ 2 * PI / 6, 0, player_motion.angle });
                m_camera.set_rotation({ PI / 2, 0, player_motion.angle});
                
                const auto temp = m_camera.rotate_to_camera_direction({ 0, 0, -1 });
                cam_dir = { temp.x, temp.y };
                ortho_cam_dir = glm::normalize(glm::cross(temp, {0, 0, 1}));
                cam_dir = Common::normalize(cam_dir);
                the_3d_angle = PI / 2;
                m_renderer->lock_cursor();
            }

            bool is_dodging = false;
            bool rotate_hero_to_velocity_dir = false;
            bool rotate_opposite_hero_to_velocity_dir = false;
            if (reg.in_dodges.has(reg.player)) {
                hero.play_animation("Roll.dae", 2.0f, false, true);
            } else if (reg.attack_cooldowns.has(reg.player)) {
                if (glm::length(player_motion.velocity) > 0.0f) {
                    hero.play_animation("Running Attack.dae", 4.0f, false, true);
                } else {
                    hero.play_animation("Standing Attack.dae", 3.0f, false, true);
                }
            } else if (reg.input_state.w_down && !reg.input_state.s_down) {
                hero.play_animation("Forward.dae");
                rotate_hero_to_velocity_dir = true;
            } else if (reg.input_state.a_down && !reg.input_state.d_down && !reg.input_state.s_down) {
                hero.play_animation("Left.dae");
            } else if (!reg.input_state.a_down && reg.input_state.d_down && !reg.input_state.s_down) {
                hero.play_animation("Right.dae");
            } else if (!reg.input_state.w_down && reg.input_state.s_down) {
                hero.play_animation("Backward.dae");
                rotate_opposite_hero_to_velocity_dir = true;
            } else {
                hero.play_animation("default0");
            }

            if (hero.get_current_animation_id() == hero.get_animation_id("Roll.dae")) {
                rotate_hero_to_velocity_dir = true;
                is_dodging = true;
            } else if (hero.get_current_animation_id() == hero.get_animation_id("Running Attack.dae")) {
                rotate_hero_to_velocity_dir = true;
                is_dodging = true; // hacky way to get cinematic effect on running attack
            }
            hero.set_position(glm::vec3(player_motion.position, 0.0f));

            const glm::vec3 desired_camera_pos = glm::vec3(player_motion.position - (cam_dir * 3.0f), 3.5f) + (1.2f * ortho_cam_dir);
            glm::vec3 current_camera_position = m_camera.get_position();
            float dist_from_desired_pos = glm::distance(desired_camera_pos, current_camera_position);
            glm::vec3 dir_ortho_to_player = glm::normalize(glm::cross(glm::vec3(Transform::create_rotation_matrix({0, 0, player_motion.angle}) * glm::vec4(0, 1, 0, 0)), glm::vec3(0, 0, 1)));
            glm::vec3 dir_to_look = glm::normalize(glm::vec3(player_motion.position, 3.5f) + 1.5f * glm::vec3(cam_dir, 0.0f) + (1.2f * dir_ortho_to_player) - current_camera_position);
            m_camera.set_rotation({ PI / 2, 0, _vector_to_angle(glm::vec2(dir_to_look)) - PI / 2});
            float amount_to_move = fmin(dist_from_desired_pos, camera_speed);
            if (is_dodging) {
                float portion_complete = hero.get_portion_complete_of_curr_animation();
                camera_speed = portion_complete * base_camera_speed;
            } else {
                camera_speed = base_camera_speed;
            }
            if (rotate_hero_to_velocity_dir && glm::length(player_motion.velocity) > 0.001f) {
                hero.set_rotation_z(_vector_to_angle(player_motion.velocity) + PI / 2);
            } else if (rotate_opposite_hero_to_velocity_dir && glm::length(player_motion.velocity) > 0.001f) {
                hero.set_rotation_z(_vector_to_angle(-player_motion.velocity) + PI / 2);
            } else {
                hero.set_rotation_z(player_motion.angle + PI);
            }
            if (amount_to_move < 0.000001f) {
                m_camera.set_position(desired_camera_pos);
            } else {
                m_camera.set_position(current_camera_position + amount_to_move * glm::normalize(desired_camera_pos - current_camera_position));
            }

            hero.update();
            _update_models();

            // _handle_free_camera_inputs();
            m_light_pos = m_camera.get_position();
            
            animated_shader.set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
            animated_shader.set_uniform_3f("u_view_pos", m_camera.get_position());
            animated_shader.set_uniform_3f("u_light_pos", m_light_pos);
            animated_shader.set_uniform_3f("u_light_color", m_light_colour);
            animated_shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            static_shader.set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
            static_shader.set_uniform_3f("u_view_pos", m_camera.get_position());
            static_shader.set_uniform_3f("u_light_pos", m_light_pos);
            static_shader.set_uniform_3f("u_light_color", m_light_colour);
            static_shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });   
            static_shader.set_uniform_1i("u_use_repeating_pattern", false);
            static_shader.set_uniform_1i("u_has_texture", true);
            static_shader.set_uniform_1i("u_has_vertex_colors", false);


            m_renderer->begin_draw();
            _draw_map_and_skybox();
            _draw_walls();
            
            hero.draw();
            
            for (const auto& kv : m_models) {
                kv.second->draw();
            }

            m_renderer->end_draw();
            time_of_last_frame = float(timer.GetTime());
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

#pragma region Setup for Square
        float square_vertices[] = {
            -0.5, -0.5, 0,   0, 0, // 0
             0.5, -0.5, 0,   1, 0, // 1
             0.5,  0.5, 0,   1, 1, // 2
            -0.5,  0.5, 0,   0, 1, // 3
        };
        unsigned int square_indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        IndexBuffer square_ibo(square_indices, Common::c_arr_count(square_indices));
        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(2); // uv
        VertexArray square_vao;
        square_vao.init();
        VertexBuffer square_vbo(square_vertices, sizeof(square_vertices));
        square_vao.add_buffer(square_vbo, layout);
#pragma endregion

#pragma region Setup for Cube
        float cube_vertices[] = {
            // Front face (Z = -0.5)
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            // Back face (Z = 0.5)
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,

            // Left face (X = -0.5)
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            // Right face (X = 0.5)
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,

            // Top face (Y = 0.5)
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,

            // Bottom face (Y = -0.5)
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f
        };

        unsigned int cube_indices[] = {
            0,  1,  2,  2,  3,  0,  // Front face
            4,  5,  6,  6,  7,  4,  // Back face
            8,  9,  10, 10, 11, 8,  // Left face
            12, 13, 14, 14, 15, 12, // Right face
            16, 17, 18, 18, 19, 16, // Top face
            20, 21, 22, 22, 23, 20  // Bottom face
        };
        IndexBuffer cube_ibo(cube_indices, Common::c_arr_count(cube_indices));
        VertexArray cube_vao;
        cube_vao.init();
        VertexBuffer cube_vbo(cube_vertices, sizeof(cube_vertices));
        cube_vao.add_buffer(cube_vbo, layout);
#pragma endregion

        // World map setup.
        TextureMaster& master = TextureMaster::get_instance();
        // TextureInfo map_texture_info = master.get_texture("disnie_map.jpg");
        // TextureInfo map_texture_info = master.get_texture_info("grass.jpg");
        // TextureInfo map_texture_info = master.get_texture_info("8bit_flower_garden.jpg");
        TextureInfo map_texture_info = master.get_texture_info("jungle_tile_1.jpg");
        Shader shader("MapDemo");

        // TextureInfo carpet_texture_info = master.get_texture_info("tileset_7.png");
        TextureInfo carpet_texture_info = master.get_texture_info("tileset_4.png");

        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        m_camera.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });

        World world;
        world.demo_init();
        Registry& reg = Registry::get_instance();
        
        Timer timer;
        float time_of_last_frame = float(timer.GetTime());

        Shader health_shader("MapDemoHealth");
        health_shader.bind();

        Shader skybox_shader("Skybox");
        skybox_shader.bind();

        Texture2D* carpet_texture = master.get_texture(carpet_texture_info.texture_slot_id);
        carpet_texture->enable_wrapping();

        Texture2D* map_texture = master.get_texture(map_texture_info.texture_slot_id);
        map_texture->enable_wrapping();


        TextureInfo skybox_info = master.load_skybox("random_skybox.png");

        bool is_cursor_locked = false;
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
                m_camera.set_rotation({ 2 * PI / 6, 0, player_motion.angle });
                const auto temp = m_camera.rotate_to_camera_direction({ 0, 0, -1 });
                cam_dir = { temp.x, temp.y };
                cam_dir = Common::normalize(cam_dir);
                m_camera.set_position(glm::vec3(player_motion.position - (cam_dir * 5.0f), 7));
                the_3d_angle = PI / 2;
                if (!is_cursor_locked) { renderer.lock_cursor(); is_cursor_locked = true; }
            } else {
                m_camera.set_position(glm::vec3(player_motion.position, CAMERA_DISTANCE_FROM_WORLD));
                m_camera.set_rotation({ 0, 0, player_motion.angle });
                if (is_cursor_locked) { renderer.unlock_cursor(); is_cursor_locked = false; }
            }

            // _handle_free_camera_inputs(renderer, cam);
            renderer.begin_draw();

            // Render skybox.
            {
                // Stinky code that I'm using just to get things to work. Needs to be refactored:
                GL_Call(glDepthFunc(GL_LEQUAL));
                skybox_shader.set_uniform_mat4f(
                    "u_view_project", 
                    m_camera.get_view_project_matrix() 
                    * Transform::create_scaling_matrix({ 500, 500, 500 })
                );
                skybox_shader.set_uniform_1i("u_skybox", skybox_info.texture_slot_id);
                renderer.draw(cube_vao, cube_ibo, skybox_shader);
                GL_Call(glDepthFunc(GL_LESS));
            }

            // Render world
            {
                shader.set_uniform_mat4f(
                    "u_mvp", 
                    m_camera.get_view_project_matrix()
                    * Transform::create_scaling_matrix({ MAP_WIDTH, MAP_HEIGHT, 1 })
                );
                shader.set_uniform_1i("u_texture", map_texture_info.texture_slot_id);
                shader.set_uniform_3f("u_scale", { MAP_WIDTH / 8, MAP_HEIGHT / 8, 1});
                renderer.draw(square_vao, square_ibo, shader);

                shader.set_uniform_mat4f(
                    "u_mvp", 
                    m_camera.get_view_project_matrix()
                    * Transform::create_translation_matrix({ 0, 0, 0.01 }) * Transform::create_scaling_matrix({ 2*15, 2*15, 1 })
                );
                shader.set_uniform_1i("u_texture", carpet_texture_info.texture_slot_id);
                shader.set_uniform_3f("u_scale", { 2*15, 2*15, 1});
                renderer.draw(square_vao, square_ibo, shader);
            }

            // this is my hacky way of making the floor textures repeat, and nothing else.
            shader.set_uniform_3f("u_scale", { 1, 1, 1 });

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
                    const TextureInfo texture_info = master.get_texture_info(tex_name.name);

                    // If the entity is a wall... Use the cube geomety... We should really just use a tag, this is so hacky.
                    if (reg.teams.has(textured_entity) && reg.teams.get(textured_entity).team_id == TEAM_ID::NEUTRAL) {
                        if (!reg.rotate_withs.has(textured_entity)) {
                            shader.set_uniform_1i("u_texture", texture_info.texture_slot_id);
                            shader.set_uniform_mat4f(
                                "u_mvp", 
                                m_camera.get_view_project_matrix()
                                * Transform::create_model_matrix(
                                    { motion.position.x, motion.position.y, motion.scale.y / 2 - 0.01},
                                    { 0, 0, motion.angle },
                                    { motion.scale.x, motion.scale.y, 3 }
                                )
                            );
                            renderer.draw(cube_vao, cube_ibo, shader);
                            continue; // Just go to next entity when you are done. There is nothing else to do lmao.
                        }
                    }

                    float z_index = 0.1 + (i++) * or_something;
                    // If the weapon is being held, I want it to come in front of the player
                    // that is holding it. There is also code for 3d-mode in here to adjust the
                    // position of the sword.
                    if (reg.weapons.has(textured_entity)) {
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
                    
                    // Now we use whatever we found to render the entity... This is going to get messy...
                    shader.set_uniform_1i("u_texture", texture_info.texture_slot_id);
                    if (Globals::is_3d_mode) {
                        shader.set_uniform_mat4f(
                            "u_mvp", 
                            m_camera.get_view_project_matrix() * Transform::create_model_matrix(
                                glm::vec3(motion_pos.x - z_index * cam_dir.x, motion_pos.y - z_index * cam_dir.y, motion.scale.y / 2), 
                                { the_3d_angle, 0, motion.angle }, 
                                glm::vec3(motion.scale, 1.0)
                            )
                        );


                    } else {
                        shader.set_uniform_mat4f(
                            "u_mvp", 
                            m_camera.get_view_project_matrix() * Transform::create_model_matrix(
                                glm::vec3(motion.position, z_index), 
                                { 0, 0, motion.angle }, 
                                glm::vec3(motion.scale, 1.0)
                            )
                        );
                    }
                    renderer.draw(square_vao, square_ibo, shader);
                }
            }

            // Render Health Bars
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
                            const auto& temp = Transform::create_rotation_matrix({ m_camera.get_rotation().x - PI / 2, m_camera.get_rotation().y, m_camera.get_rotation().z }) * glm::vec4(0, 0, -1, 1);
                            health_bar_pos += (2.5f * glm::vec3(temp.x, temp.y, temp.z)) + (3.0f * cam_dir_3d);
                        } else {
                            health_bar_pos = glm::vec3({ loco_motion.position.x, loco_motion.position.y, loco_motion.scale.y + HEALTH_BAR_HEIGHT / 2 + 0.5});
                        }
                            health_bar_angle = m_camera.get_rotation();

                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            m_camera.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                health_bar_pos,
                                health_bar_angle,
                                glm::vec3({ loco_motion.scale.x, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 1, 0, 0 });
                        renderer.draw(square_vao, square_ibo, health_shader);

                        // Green health bar layer
                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            m_camera.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                health_bar_pos - (0.001f * m_camera.rotate_to_camera_direction({ 0, 0, -1 })),
                                health_bar_angle,
                                glm::vec3({ loco_motion.scale.x * health_percentage, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 0, 1, 0 });
                        renderer.draw(square_vao, square_ibo, health_shader);
                    } else {
                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            m_camera.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                glm::vec3({ loco_motion.position.x, loco_motion.position.y - loco_motion.scale.y / 2 - HEALTH_BAR_HEIGHT / 2, z_index }),
                                glm::vec3({ 0, 0, 0 }),
                                glm::vec3({ loco_motion.scale.x, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 1, 0, 0 });
                        renderer.draw(square_vao, square_ibo, health_shader);

                        z_index += 0.001;
                        // Green health bar layer
                        health_shader.set_uniform_mat4f(
                            "u_mvp",
                            m_camera.get_view_project_matrix()
                            * Transform::create_model_matrix(
                                glm::vec3({ loco_motion.position.x, loco_motion.position.y - loco_motion.scale.y / 2 - HEALTH_BAR_HEIGHT / 2, z_index }),
                                glm::vec3({ 0, 0, Globals::is_3d_mode ? player_motion.angle : 0 }),
                                glm::vec3({ loco_motion.scale.x * health_percentage, HEALTH_BAR_HEIGHT, 1 })
                            )
                        );
                        health_shader.set_uniform_3f("u_colour", { 0, 1, 0});
                        renderer.draw(square_vao, square_ibo, health_shader);
                    }
                }
            }

            // Draw the aim trajectory for the player
            // I'm using the Health bar shader because well... I just wanted
            // to draw rectangles with static colours instead of textures...
            if (Globals::is_3d_mode){
                health_shader.set_uniform_3f("u_colour", { 1, 0, 0.5 }); // fuschia croshair (pink)
                glm::vec3 crosshair_pos = { player_motion.position.x, player_motion.position.y, player_motion.scale.y / 2.0f };
                const float crosshair_length = 1000;
                crosshair_pos += glm::vec3(cam_dir, 0) * (crosshair_length / 2.0f);
                health_shader.set_uniform_mat4f(
                    "u_mvp",
                    m_camera.get_view_project_matrix()
                    * Transform::create_model_matrix(
                        crosshair_pos,
                        glm::vec3({ 0, 0, player_motion.angle }),
                        glm::vec3({ 0.2, crosshair_length, 1 })
                    )
                );
                renderer.draw(square_vao, square_ibo, health_shader);
            }

            

            renderer.end_draw();
            time_of_last_frame = float(timer.GetTime());
        }
    }

private:
    void _handle_free_camera_inputs() {
        glm::vec3 moveDirection(0.0f);
        glm::vec3 rotateDirection(0.0f);

        float moveSpeed = 0.05f;
        float rotateSpeed = PI / 300;  // radians per frame

        glm::vec3 newPosition = 1.f * m_camera.get_position();
        glm::vec3 newRotation = 1.f * m_camera.get_rotation();
        bool pos_changed = false;
        bool rot_changed = false;

        glm::vec3 player_input(0.0f);

        // Handle keyboard input for movement
        if (m_renderer->is_key_pressed(GLFW_KEY_W)) {
            player_input.z -= moveSpeed;  // Move in negative Z
            pos_changed = true;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_S)) {
            player_input.z += moveSpeed;  // Move in positive Z
            pos_changed = true;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_A)) {
            player_input.x -= moveSpeed;  // Move in negative X
            pos_changed = true;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_D)) {
            player_input.x += moveSpeed;  // Move in positive X
            pos_changed = true;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_SPACE)) {
            player_input.y += moveSpeed;  // Move in positive Y
            pos_changed = true;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_LEFT_CONTROL)) {
            player_input.y -= moveSpeed;  // Move in negative Y
            pos_changed = true;
        }

        // Handle keyboard input for rotation
        if (m_renderer->is_key_pressed(GLFW_KEY_UP)) {
                rot_changed = true;
                newRotation.x += rotateSpeed;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_DOWN)) {
                rot_changed = true;
                newRotation.x -= rotateSpeed;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_LEFT)) {
                rot_changed = true;
                newRotation.z += rotateSpeed;
        }
        if (m_renderer->is_key_pressed(GLFW_KEY_RIGHT)) {
                rot_changed = true;
                newRotation.z -= rotateSpeed;
        }

        // Update camera position and rotation using the setter methods
        if (rot_changed) {
            m_camera.set_rotation(newRotation);
        }
        if (pos_changed) {
            newPosition += m_camera.rotate_to_camera_direction(player_input);
            m_camera.set_position(newPosition);
        }
    }

    std::vector<float> m_cube_vertices = {
            // Front face (Z = -0.5)
           -0.5f,  0.5f, -0.5f,         0, 0, -1,        1, 0, 0, 0,      0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,         0, 0, -1,        1, 0, 0, 0,      1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,         0, 0, -1,        1, 0, 0, 0,      1.0f, 0.0f,
           -0.5f, -0.5f, -0.5f,         0, 0, -1,        1, 0, 0, 0,      0.0f, 0.0f,

            // Back face (Z = 0.5)
           -0.5f,  0.5f,  0.5f,         0, 0, 1,        1, 0, 0, 0,        1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,         0, 0, 1,        1, 0, 0, 0,        0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,         0, 0, 1,        1, 0, 0, 0,        0.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,         0, 0, 1,        1, 0, 0, 0,        1.0f, 0.0f,

            // Left face (X = -0.5)
           -0.5f,  0.5f,  0.5f,         -1, 0, 0,        1, 0, 0, 0,        0.0f, 1.0f,
           -0.5f,  0.5f, -0.5f,         -1, 0, 0,        1, 0, 0, 0,        1.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,         -1, 0, 0,        1, 0, 0, 0,        1.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,         -1, 0, 0,        1, 0, 0, 0,        0.0f, 0.0f,

            // Right face (X = 0.5)
            0.5f,  0.5f,  0.5f,         1, 0, 0,        1, 0, 0, 0,        1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,         1, 0, 0,        1, 0, 0, 0,        0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,         1, 0, 0,        1, 0, 0, 0,        0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,         1, 0, 0,        1, 0, 0, 0,        1.0f, 0.0f,

            // Top face (Y = 0.5)
           -0.5f,  0.5f,  0.5f,         0, 1, 0,        1, 0, 0, 0,        0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,         0, 1, 0,        1, 0, 0, 0,        1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,         0, 1, 0,        1, 0, 0, 0,        1.0f, 0.0f,
           -0.5f,  0.5f, -0.5f,         0, 1, 0,        1, 0, 0, 0,        0.0f, 0.0f,

            // Bottom face (Y = -0.5)
           -0.5f, -0.5f,  0.5f,         0, -1, 0,        1, 0, 0, 0,        0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,         0, -1, 0,        1, 0, 0, 0,        1.0f, 0.0f,
            0.5f, -0.5f, -0.5f,         0, -1, 0,        1, 0, 0, 0,        1.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,         0, -1, 0,        1, 0, 0, 0,        0.0f, 1.0f
        };
    std::vector<unsigned int> m_cube_indices = {
        0,  1,  2,  2,  3,  0,  // Front face
        4,  5,  6,  6,  7,  4,  // Back face
        8,  9,  10, 10, 11, 8,  // Left face
        12, 13, 14, 14, 15, 12, // Right face
        16, 17, 18, 18, 19, 16, // Top face
        20, 21, 22, 22, 23, 20  // Bottom face
    };
    std::vector<float> m_square_vertices = {// pos, norm, colour, texcoord
        -0.5, -0.5, 0,    0, 0, 1,    1, 0, 0, 0,      0, 0, // 0
        0.5, -0.5, 0,     0, 0, 1,    1, 0, 0, 0,      1, 0,  // 1
        0.5,  0.5, 0,     0, 0, 1,    1, 0, 0, 0,      1, 1,  // 2
        -0.5,  0.5, 0,    0, 0, 1,    1, 0, 0, 0,      0, 1, // 3
    };
    std::vector<unsigned int> m_square_indices = {
        0, 1, 2,
        0, 2, 3
    };

    void _draw_map_and_skybox() {
        // Render skybox.
        GL_Call(glDepthFunc(GL_LEQUAL));
        m_skybox_shader->set_uniform_mat4f(
            "u_view_project", 
            m_camera.get_view_project_matrix() 
            * Transform::create_scaling_matrix({ 500, 500, 500 })
        );
        m_renderer->draw(m_cube_mesh, *m_skybox_shader);
        GL_Call(glDepthFunc(GL_LESS));


        m_floor_shader->set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
        m_floor_shader->set_uniform_3f("u_view_pos", m_camera.get_position());
        m_floor_shader->set_uniform_3f("u_light_pos", m_light_pos);
        m_floor_shader->set_uniform_3f("u_light_color", m_light_colour);
        m_floor_shader->set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });
        m_floor_shader->set_uniform_mat4f(
            "u_view_project", 
            m_camera.get_view_project_matrix()
        );
        m_floor_shader->set_uniform_mat4f(
            "u_model",
            Transform::create_scaling_matrix({ MAP_WIDTH, MAP_HEIGHT, 1 })
        );
        m_floor_shader->set_uniform_1i("u_texture", m_map_texture->bind(30));
        m_floor_shader->set_uniform_3f("u_scale", { MAP_WIDTH / 8, MAP_HEIGHT / 8, 1});
        m_floor_shader->set_uniform_1i("u_use_repeating_pattern", true);
        m_floor_shader->set_uniform_1i("u_has_texture", true);
        m_floor_shader->set_uniform_1i("u_has_vertex_colors", false);

        m_renderer->draw(m_square_mesh, *m_floor_shader);
    }

    void _draw_walls() {
        m_wall_shader->set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
        m_wall_shader->set_uniform_3f("u_view_pos", m_camera.get_position());
        m_wall_shader->set_uniform_3f("u_light_pos", m_light_pos);
        m_wall_shader->set_uniform_3f("u_light_color", m_light_colour);
        m_wall_shader->set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });
        m_wall_shader->set_uniform_1i("u_use_repeating_pattern", true);
        m_wall_shader->set_uniform_1i("u_has_texture", true);
        m_wall_shader->set_uniform_1i("u_has_vertex_colors", false);
        m_wall_shader->set_uniform_1i("u_texture", m_wall_texture->bind(29));
        m_wall_shader->set_uniform_mat4f(
            "u_view_project", 
            m_camera.get_view_project_matrix()
        );

        auto& reg = Registry::get_instance();
        for (auto& entity : reg.walls.entities) {
            if (!reg.motions.has(entity)) { continue; }
            auto& motion = reg.motions.get(entity);
            glm::vec3 wall_scale = glm::vec3(motion.scale, 20.0f);
            m_wall_shader->set_uniform_3f("u_scale", wall_scale);
            m_wall_shader->set_uniform_mat4f(
                "u_model",
                Transform::create_model_matrix(
                    glm::vec3(motion.position, 0.0f),
                    { 0, 0, motion.angle },
                    wall_scale
                )
            );
            m_renderer->draw(m_cube_mesh, *m_wall_shader);
        }

        m_spooky_tree->set_scale(0.15f, 0.15f, 0.35f);
        for (auto& entity : reg.static_objects.entities) {
            if (!reg.motions.has(entity)) { continue; }
            auto& static_object = reg.static_objects.get(entity);
            if (static_object.type != STATIC_OBJECT_TYPE::TREE) { continue; }
            auto& motion = reg.motions.get(entity);
            m_spooky_tree->set_position(glm::vec3(motion.position, -0.3f));
            m_spooky_tree->draw();
        }
    }

    void _update_models() {
        auto& reg = Registry::get_instance();
        for (auto& kv : m_models) {
            auto& model = kv.second;
            if (model == nullptr) {
                continue;
            }
            auto id = kv.first;
            Entity entity = reinterpret_cast<Entity&>(id);
            if (!reg.motions.has(entity)) {
                delete model;
                kv.second = nullptr;
                continue;
            }
            auto& motion = reg.motions.get(id);
            auto& player_motion = reg.motions.get(reg.player);

            float pre_rotate = 3 * PI / 2;
            auto angle = std::fabs(std::fmod(motion.angle + pre_rotate, 2 * PI));
            auto velocity_angle = _vector_to_angle(motion.velocity);
            float angle_between_view_and_velo = 
                glm::acos(
                    glm::dot(
                        glm::normalize(
                            glm::vec2(Transform::create_rotation_matrix({0, 0, angle}) * glm::vec4(1, 0, 0, 0))
                        ),
                        glm::normalize(motion.velocity)
                    )
                );

            bool is_dodging = false;
            bool rotate_to_velocity_dir = false;
            bool rotate_opposite_to_velocity_dir = false;
            if (reg.in_dodges.has(entity)) {
                model->play_animation("Roll.dae", 2.0f, false, true);
            } else if (reg.attack_cooldowns.has(entity)) {
                if (glm::length(motion.velocity) > 0.0f) {
                    model->play_animation("Running Attack.dae", 4.0f, false, true);
                } else {
                    model->play_animation("Standing Attack.dae", 3.0f, false, true);
                }
            } else if (glm::length(motion.velocity) > 0.0f) {
                
                if (angle_between_view_and_velo < PI / 3) {
                    model->play_animation("Forward.dae");
                } else if (angle_between_view_and_velo > 2 * PI / 3) {
                    model->play_animation("Backward.dae");
                } else if (
                    (velocity_angle - angle < PI && velocity_angle - angle >= 0)
                    || velocity_angle - angle < -PI && velocity_angle - angle < 0) {
                    model->play_animation("Left.dae");
                } else {
                    model->play_animation("Right.dae");
                }

            } else {
                model->play_animation("default0");
            }

            const auto current_anim_id = model->get_current_animation_id();
            if (current_anim_id == model->get_animation_id("Roll.dae")) {
                rotate_to_velocity_dir = true;
            } else if (current_anim_id == model->get_animation_id("Running Attack.dae")) {
                rotate_to_velocity_dir = true;
            } else if (current_anim_id == model->get_animation_id("Forward.dae")) {
                rotate_to_velocity_dir = true;
            } else if (current_anim_id == model->get_animation_id("Backward.dae")) {
                rotate_opposite_to_velocity_dir = true;
            }
            model->set_position(glm::vec3(motion.position, 0.0f));

            if (rotate_to_velocity_dir) {
                model->set_rotation_z(velocity_angle - 3 * PI / 2);
            } else if (rotate_opposite_to_velocity_dir) {
                model->set_rotation_z(velocity_angle - PI / 2);
            } else {
                model->set_rotation_z(motion.angle);
            }

            model->update();
        }
    }

    //helpers
    float _vector_to_angle(const glm::vec2& vec) {
        float angle = acos(glm::dot({1, 0}, glm::normalize(vec)));
        if (vec.y < 0) {
            angle = 2 * M_PI - angle;
        }
        return angle;
    }
};
