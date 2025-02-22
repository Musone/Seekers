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
#include <renderer/FontStuff.hpp>
#include <renderer/Menu.hpp>
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
#include <memory>
#include <functional>

#define MAX_LIGHTS 25

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
    StaticModel* m_light_orb;
    StaticModel* m_campfire;
    StaticModel* m_dungeon_entrance;
    StaticModel* m_portal;
    StaticModel* m_sword;
    StaticModel* m_bow;
    StaticModel* m_arrow;
    StaticModel* m_banana;
    std::vector<StaticModel*> m_rocks;

    Texture2D* m_map_texture;
    Shader* m_floor_shader;

    Shader* m_health_shader;
    Shader* m_hud_health_shader;
    Texture2D* m_hud_health_texture_fill;
    Texture2D* m_hud_health_texture_border;
    Texture2D* m_hud_health_texture_bacground;
    Texture2D* m_redbull;
    Texture2D* m_lock_on_reticle;
    Texture2D* m_loading_tex;
    Texture2D* m_home_tex;

    glm::vec3 m_light_pos;
    glm::vec3 m_light_colour;

    std::vector<int> m_to_be_updated_and_drawn;
    std::vector<glm::vec3> m_light_positions;
    std::vector<glm::vec3> m_light_colours;
    std::vector<float> m_light_brightnesses;
    std::vector<StaticModel*> m_light_models;

    std::string m_window_name = "Seekers";

    std::unordered_map<unsigned int, AnimatedModel*> m_models;
    bool m_player_was_in_rest = false;

    std::unique_ptr<Menu> main_menu;
    std::unordered_map<std::string, Texture2D*> m_menu_textures;
    bool m_game_in_session = false;

    float m_frame_rate = 0.0f;
public:
    Application() : m_light_pos(1.0f, 1.0f, 2.0f) {
        // Setup
        m_renderer = &Renderer::get_instance();
        m_renderer->init(
            m_window_name,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            true,
            false,
            true
        );
        m_renderer->set_icon("textures/favicon.png");
        m_camera.init(m_renderer->get_window_width(), m_renderer->get_window_height());

        m_hud_health_texture_fill = new Texture2D("sphere_fill.png");
        m_hud_health_texture_border = new Texture2D("sphere_border.png");
        m_hud_health_texture_bacground = new Texture2D("sphere_background.png");
        m_redbull = new Texture2D("Redbull.png");
        m_lock_on_reticle = new Texture2D("lock_on_reticle.png");

        m_skybox_shader = new Shader("Skybox");
        // m_skybox_texture = new SkyboxTexture("random_skybox.png"); // The red lava skybox
        m_skybox_texture = new SkyboxTexture("Blue sky.png");
        m_map_texture = new Texture2D("jungle_tile_1.jpg");
        // m_wall_texture = new Texture2D("tileset_1.png"); // bricks
        m_wall_texture = new Texture2D("jungle_tile_1.jpg");
        // m_loading_tex = new Texture2D("menu/seekers_background.JPG");
        m_loading_tex = new Texture2D("menu/loading.JPG");
        // m_home_tex =  new Texture2D("menu/home.jpg");
        m_menu_textures["play"] = new Texture2D("menu/play.jpg");
        m_menu_textures["hover_play"] = new Texture2D("menu/hover_play.jpg");
        m_menu_textures["load"] = new Texture2D("menu/load.jpg");
        m_menu_textures["hover_load"] = new Texture2D("menu/hover_load.jpg");
        m_menu_textures["quit"] = new Texture2D("menu/quit.jpg");
        m_menu_textures["hover_quit"] = new Texture2D("menu/hover_quit.jpg");
        m_menu_textures["resume"] = new Texture2D("menu/resume.jpg");
        m_menu_textures["hover_resume"] = new Texture2D("menu/hover_resume.jpg");
        
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

        m_spooky_tree = new StaticModel("models/Spooky Tree/Spooky Tree.obj", m_wall_shader);
        m_spooky_tree->m_has_texture = true;
        m_spooky_tree->texture_list.push_back(std::make_shared<Texture2D>("Spooky Tree.jpg"));
        m_spooky_tree->mesh_list.back()->set_texture(m_spooky_tree->texture_list.back());
        m_spooky_tree->set_pre_transform(
            Transform::create_scaling_matrix(glm::vec3(0.15f, 0.15f, 0.35f))
        );

        m_light_orb = new StaticModel("models/Orb_low.obj", m_wall_shader);
        m_light_orb->m_has_texture = true;
        m_light_orb->texture_list.push_back(std::make_shared<Texture2D>("Orb_low_piedra.001_Emissive.png"));
        m_light_orb->mesh_list.back()->set_texture(m_light_orb->texture_list.back());

        m_campfire = new StaticModel("models/Campfire.obj", m_wall_shader);
        m_campfire->m_has_texture = true;
        m_campfire->texture_list.push_back(std::make_shared<Texture2D>("Campfire_MAT_BaseColor_01.jpg"));
        m_campfire->mesh_list.back()->set_texture(m_campfire->texture_list.back());
        m_campfire->set_scale(glm::vec3(0.0375f));
        m_campfire->set_pre_transform(Transform::create_rotation_matrix({PI / 2.0f, 0, 0}));

        m_dungeon_entrance = new StaticModel("models/drenn_entrance_b.stl", m_wall_shader);
        m_dungeon_entrance->set_scale(glm::vec3(2));
        m_dungeon_entrance->set_pre_transform(
            Transform::create_model_matrix(
                {0, 0, -6},
                {0, 0, 0},
                glm::vec3(0.5f)
            )
        );
        m_portal = new StaticModel("models/Portal/portal.obj", m_wall_shader);
        m_portal->set_pre_transform(
            Transform::create_model_matrix(
                {0, 0, -1.1},
                {PI / 2.0f, 0, PI},
                glm::vec3(1.0f)
            )
        );
        // m_dungeon_entrance = ;
        // m_dungeon_entrance

        m_rocks.push_back(new StaticModel("models/rocks/CaveRock01_A.dae", m_wall_shader));
        m_rocks.push_back(new StaticModel("models/rocks/CaveRock01_B.dae", m_wall_shader));
        m_rocks.push_back(new StaticModel("models/rocks/CaveRock01_C.dae", m_wall_shader));
        m_rocks.push_back(new StaticModel("models/rocks/CaveRock01_D.dae", m_wall_shader));
        m_rocks.push_back(new StaticModel("models/rocks/CaveRock01_E.dae", m_wall_shader));
        for (auto& rock : m_rocks) {
            rock->m_has_texture = true;
            rock->texture_list.push_back(std::make_shared<Texture2D>("CaveRockOld_Diffuse.png"));
            rock->mesh_list.back()->set_texture(rock->texture_list.back());
        }
        m_rocks[1]->set_pre_transform(Transform::create_translation_matrix({2.0f, 2.5f, 0})); // Stinky hardcode compensation for model and bounding box not aligning.

        m_bow = new StaticModel("models/Bow.obj", m_wall_shader);
        m_sword = new StaticModel("models/Sword.obj", m_wall_shader);

        m_arrow = new StaticModel("models/Arrow.dae", m_wall_shader);
        m_arrow->set_scale(glm::vec3(5));
        m_arrow->set_pre_transform(
            Transform::create_rotation_matrix({21.1533680, 22.0539455, 116.577499})
        );
        m_banana = new StaticModel("models/Melee.obj", m_wall_shader);
        m_banana->set_pre_transform(
            Transform::create_rotation_matrix({0, 0, - PI / 2}) *
            Transform::create_scaling_matrix(glm::vec3(0.03, 0.03, 0.05))
        );

        m_light_colour = glm::vec3(1.0f);

        m_health_shader = new Shader("MapDemoHealth");
        m_hud_health_shader = new Shader("TexturedHealthBar");

        FontStuff& font_monkey = FontStuff::get_instance();
        font_monkey.font_init("fonts/Cano-VGMwz.ttf", 42, m_renderer->get_window_width(), m_renderer->get_window_height());

        // Registry& registry = MapManager::get_instance().get_active_registry();
        // auto& models = registry.projectile_models.emplace(Entity());
        // models.arrow_model = m_arrow;
        // models.melee_model = m_banana;
    }

    ~Application() {
        delete m_skybox_shader;
        delete m_skybox_texture;
        delete m_map_texture;
        delete m_floor_shader;
    }

    void run_game_loop() { 
        _draw_loading_screen();
        
        // Get keys inputs from input manager
        m_renderer->set_on_key_callback_fn((void*)InputManager::on_key_pressed);
        m_renderer->set_on_mouse_move_callback_fn((void*)InputManager::on_mouse_move);
        m_renderer->set_on_mouse_click_callback_fn((void*)InputManager::on_mouse_button_pressed);

        Shader animated_shader("AnimatedBlinnPhong");
        Shader static_shader("StaticBlinnPhong");

#pragma region HERO
        AnimatedModel hero("models/Hero/Hero.dae", &animated_shader);
        hero.load_animation_from_file("models/Hero/Left.dae");
        hero.load_animation_from_file("models/Hero/Right.dae");
        hero.load_animation_from_file("models/Hero/Backward.dae");
        hero.load_animation_from_file("models/Hero/Forward.dae");
        hero.load_animation_from_file("models/Hero/Roll.dae");
        hero.load_animation_from_file("models/Hero/Standing Attack.dae");
        hero.load_animation_from_file("models/Hero/Running Attack.dae");
        hero.load_animation_from_file("models/Hero/Dying.dae");
        hero.load_animation_from_file("models/Hero/Stagger.dae");
        hero.load_animation_from_file("models/Dance.dae");
        hero.load_animation_from_file("models/Hero/Sitting.dae");
        hero.load_animation_from_file("models/Hero/Stand Up.dae");

        // AnimatedModel hero("models/Hero/Hero (no sword).dae", &animated_shader);
        // hero.load_animation_from_file("models/Hero/Left.dae");
        // hero.load_animation_from_file("models/Hero/Right.dae");
        // hero.load_animation_from_file("models/Hero/Backward.dae");
        // hero.load_animation_from_file("models/Hero/Forward.dae");
        // hero.load_animation_from_file("models/Hero/Roll.dae");
        // hero.load_animation_from_file("models/Archer Grunt/Standing Attack.dae");
        // hero.load_animation_from_file("models/Archer Grunt/Running Attack.dae");
        // hero.load_animation_from_file("models/Archer Grunt/Dying.dae");
        // hero.load_animation_from_file("models/Archer Grunt/Stagger.dae");
        hero.set_pre_transform(
            Transform::create_model_matrix(
                glm::vec3(0),
                glm::vec3(PI / 2, 0, PI / 2),
                glm::vec3(0.02)
            )
        );
        hero.print_bones();
        hero.print_animations();
#pragma endregion

#pragma region WARRIOR
        AnimatedModel warrior_grunt("models/Warrior Grunt/Warrior Grunt (drake).dae", &animated_shader);
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Left.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Right.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Backward.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Forward.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Roll.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Standing Attack.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Running Attack.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Dying.dae");
        warrior_grunt.load_animation_from_file("models/Warrior Grunt/Stagger.dae");
        warrior_grunt.load_animation_from_file("models/Dance.dae");
        warrior_grunt.set_pre_transform(
            Transform::create_model_matrix(
                glm::vec3(0),
                glm::vec3(PI / 2, 0, PI / 2),
                glm::vec3(0.02)
            )
        );
        warrior_grunt.print_bones();
        warrior_grunt.print_animations();
#pragma endregion

#pragma region ARCHER
        AnimatedModel archer_grunt("models/Archer Grunt/Archer Grunt.dae", &animated_shader);
        archer_grunt.load_animation_from_file("models/Archer Grunt/Left.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Right.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Backward.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Forward.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Roll.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Standing Attack.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Running Attack.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Dying.dae");
        archer_grunt.load_animation_from_file("models/Archer Grunt/Stagger.dae");
        archer_grunt.load_animation_from_file("models/Dance.dae");
        archer_grunt.set_pre_transform(
            Transform::create_model_matrix(
                glm::vec3(0),
                glm::vec3(PI / 2, 0, PI / 2),
                glm::vec3(0.02)
            )
        );
        archer_grunt.print_bones();
        archer_grunt.print_animations();
#pragma endregion

#pragma region ZOMBIE
        AnimatedModel zombie_grunt("models/Zombie Grunt/Zombie Grunt.dae", &animated_shader);
        zombie_grunt.load_animation_from_file("models/Zombie Grunt/Forward.dae");
        zombie_grunt.load_animation_from_file("models/Zombie Grunt/Roll.dae");
        zombie_grunt.load_animation_from_file("models/Zombie Grunt/Standing Attack.dae");
        zombie_grunt.load_animation_from_file("models/Zombie Grunt/Running Attack.dae");
        zombie_grunt.load_animation_from_file("models/Zombie Grunt/Dying.dae");
        zombie_grunt.load_animation_from_file("models/Zombie Grunt/Stagger.dae");
        zombie_grunt.load_animation_from_file("models/Dance.dae");
        zombie_grunt.set_pre_transform(
            Transform::create_model_matrix(
                glm::vec3(0),
                glm::vec3(PI / 2, 0, PI / 2),
                glm::vec3(0.02)
            )
        );
        zombie_grunt.print_bones();
        zombie_grunt.print_animations();
#pragma endregion

        // Texture2D old_hero("player.png");

        // if (hero.get_animation_count() > 0) {
        //     hero.play_animation(0);
        // }

        _init_menu();

        World world;
        world.demo_init();

        Registry& regie = MapManager::get_instance().get_active_registry();
        auto& models = regie.projectile_models.emplace(Entity());
        models.arrow_model = m_arrow;
        models.melee_model = m_banana;

        AnimatedModel* player_model;

        Globals::ptr_window = m_renderer->get_window();

        Timer timer;
        float time_of_last_frame = 0;
        const float FRAME_TIME_60FPS = 1000000.0f / 60.0f;
        float base_camera_speed = 30.0f;
        float camera_speed = base_camera_speed;
        while (!m_renderer->is_terminated()) {
            // float delta_time = 0.001f * float(timer.GetTime()) - time_of_last_frame;
            // while (delta_time < 1000.0f / 60.0f) { delta_time = 0.001f * (float(timer.GetTime()) - time_of_last_frame); }
            float delta_time = float(timer.GetTime()) - time_of_last_frame;
            while (delta_time < FRAME_TIME_60FPS) {
                delta_time = float(timer.GetTime()) - time_of_last_frame;
            }
            float delta_time_s = delta_time * 0.000001f;
            m_frame_rate = 1.0f / delta_time_s;
            // m_renderer->set_title(m_window_name + " | FPS: " + std::to_string(m_frame_rate));
            time_of_last_frame = float(timer.GetTime());

            if (Globals::in_pause) {// testing menustd::unique_ptr<
                menu_update();
                main_menu->draw(m_renderer, m_hud_health_shader, m_square_mesh);
                continue;
            }

            world.step(delta_time * 0.001f);
            Registry& reg = MapManager::get_instance().get_active_registry();
            MapManager& map_monkey = MapManager::get_instance();
            // if (map_monkey.enter_dungeon_flag || map_monkey.return_open_world_flag || Globals::show_loading_screen) {
            //     // loading screen?
            //     _draw_home_page();
            //     world.step(delta_time * 0.001f);
            // }

            // Game restart
            if (Globals::restart_renderer) {
                Globals::restart_renderer = false;
                _draw_loading_screen();

                _draw_loading_screen();

                m_camera.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });
                for (auto& kv : m_models) {
                    if (kv.second == nullptr) { continue; }
                    delete kv.second;
                    kv.second = nullptr;
                }

                unsigned int counter = 1;
                m_models[reg.player.get_id()] = new AnimatedModel(hero, counter++);
                for (const auto& entity : reg.enemies.entities) {
                    const auto& enemy = reg.enemies.get(entity);
                    if (enemy.type == ENEMY_TYPE::WARRIOR) {
                        m_models[entity.get_id()] = new AnimatedModel(warrior_grunt, counter++);
                        const auto& model = m_models[entity.get_id()];
                        model->attach_to_joint(
                            m_sword,
                            "mixamorig_RightHand",
                            {63.0, 35.0, 7.0}, // pos
                            {4.7752223, 19.3731594, 11.5401363}, // rot
                            {11.5, 11.5, 11.5} // scale
                        );
                    } else if (enemy.type == ENEMY_TYPE::ARCHER) {
                        m_models[entity.get_id()] = new AnimatedModel(archer_grunt, counter++);
                        const auto& model = m_models[entity.get_id()];
                        model->attach_to_joint(
                            m_bow,
                            "mixamorig_RightHand",
                            {63.0, 35.0, -10.5}, // pos
                            {10.3044329, 14.5560884, 12.8805599}, // rot
                            {25.5, 25.5, 25.5} // scale
                        );
                    } else {
                        m_models[entity.get_id()] = new AnimatedModel(zombie_grunt, counter++);
                        const auto& model = m_models[entity.get_id()];
                    }
                }
                player_model = m_models[reg.player.get_id()];
                // delta_time = delta_time_s = 0.00000000001f;
                time_of_last_frame = float(timer.GetTime());
                _update_theme();
            }

            if (map_monkey.enter_dungeon_flag || map_monkey.return_open_world_flag) {
                // loading screen?
                _draw_loading_screen();
                continue;
            }

            if (map_monkey.enter_dungeon_flag || map_monkey.return_open_world_flag) {
                // loading screen?
                _draw_loading_screen();
                continue;
            }

            // Camera stuff
            const Motion& player_motion = reg.motions.get(reg.player);
            glm::vec2 cam_dir;
            glm::vec3 ortho_cam_dir;
            // m_camera.set_position(glm::vec3(player_motion.position, 2.0f));
            // m_camera.set_rotation({PI / 2, 0, player_motion.angle - PI / 2});
            {
                    float the_3d_angle = 0;
                    m_camera.set_rotation({ PI / 2, 0, player_motion.angle - PI / 2});

                    const auto temp = m_camera.rotate_to_camera_direction({ 0, 0, -1 });
                    cam_dir = { temp.x, temp.y };
                    ortho_cam_dir = glm::normalize(glm::cross(temp, {0, 0, 1}));
                    cam_dir = Common::normalize(cam_dir);
                    the_3d_angle = PI / 2;
                    m_renderer->lock_cursor();

                bool is_dodging = false;
                if (player_model->get_current_animation_id() == player_model->get_animation_id("Roll.dae")) {
                    is_dodging = true;
                }

                const glm::vec3 desired_camera_pos = glm::vec3(player_motion.position - (cam_dir * 3.0f), 3.5f) + (1.2f * ortho_cam_dir);
                glm::vec3 current_camera_position = m_camera.get_position();
                float dist_from_desired_pos = glm::distance(desired_camera_pos, current_camera_position);
                glm::vec3 dir_ortho_to_player = glm::normalize(
                    glm::cross(
                        glm::vec3(
                            Transform::create_rotation_matrix({0, 0, player_motion.angle}) * glm::vec4(1, 0, 0, 0)
                        ),
                        glm::vec3(0, 0, 1)
                        )
                );
                glm::vec3 dir_to_look = glm::normalize(
                    glm::vec3(player_motion.position, 3.5f) +
                    1.5f * glm::vec3(cam_dir, 0.0f) +
                    (1.2f * dir_ortho_to_player) -
                    current_camera_position
                );
                m_camera.set_rotation({ PI / 2, 0, _vector_to_angle(glm::vec2(dir_to_look)) - PI / 2});

                // if (reg.locked_target.is_active) {
                //     if (reg.motions.has(reg.locked_target.target)) {
                //         dir_to_look = glm::normalize(
                //             glm::vec3(reg.motions.get(reg.locked_target.target).position, 3.5f) -
                //             current_camera_position
                //         );
                //     }
                // }

                if (is_dodging) {
                    float portion_complete = player_model->get_portion_complete_of_curr_animation();
                    camera_speed = portion_complete * base_camera_speed * delta_time_s;
                } else {
                    camera_speed = base_camera_speed * delta_time_s;
                }
                float amount_to_move = fmin(dist_from_desired_pos, camera_speed);
                if (amount_to_move < 0.000001f) {
                    m_camera.set_position(desired_camera_pos);
                } else {
                    m_camera.set_position(current_camera_position + amount_to_move * glm::normalize(desired_camera_pos - current_camera_position));
                }
            }

            reg.camera_pos = m_camera.get_position();
            // _handle_free_camera_inputs();
            m_light_pos = m_camera.get_position();

            // Temporarily permanent code that handles the multiple light source update
            {
                m_light_positions.clear();
                m_light_brightnesses.clear();
                m_light_colours.clear();
                m_light_models.clear();

                m_light_positions.reserve(MAX_LIGHTS);
                m_light_brightnesses.reserve(MAX_LIGHTS);
                m_light_colours.reserve(MAX_LIGHTS);
                m_light_models.reserve(MAX_LIGHTS);

                m_light_positions.push_back(m_light_pos);
                m_light_brightnesses.push_back(1.0f);
                m_light_colours.push_back(m_light_colour);
                m_light_models.push_back(nullptr);
                int counter = 1;

                for (const auto& entity : reg.light_sources.entities) {
                    if (counter > MAX_LIGHTS) { break; }
                    const LightSource& light_source = reg.light_sources.get(entity);
                    if (!reg.near_cameras.has(entity) && light_source.type != LIGHT_SOURCE_TYPE::SUN) { continue; }
                    if (!reg.near_cameras.has(entity) && light_source.type != LIGHT_SOURCE_TYPE::SUN) { continue; }
                    ++counter;
                    m_light_positions.push_back(light_source.pos);
                    m_light_brightnesses.push_back(light_source.brightness);
                    m_light_colours.push_back(light_source.colour);
                    if (light_source.type == LIGHT_SOURCE_TYPE::MAGIC_ORB) {
                        m_light_models.push_back(m_light_orb);
                    } else {
                        m_light_models.push_back(nullptr);
                    }
                }
            }

            animated_shader.set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
            animated_shader.set_uniform_3f("u_view_pos", m_camera.get_position());
            // animated_shader.set_uniform_3f("u_light_pos", m_light_pos);
            // animated_shader.set_uniform_3f("u_light_color", m_light_colour);
            // animated_shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            animated_shader.set_uniform_1i("u_num_lights", m_light_positions.size());
            animated_shader.set_uniform_3f_array("u_light_positions", *m_light_positions.data(), m_light_positions.size());
            animated_shader.set_uniform_1f_array("u_light_strengths", *m_light_brightnesses.data(), m_light_brightnesses.size());
            animated_shader.set_uniform_3f_array("u_light_colours", *m_light_colours.data(), m_light_colours.size());

            static_shader.set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
            static_shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });
            static_shader.set_uniform_1i("u_use_repeating_pattern", false);
            static_shader.set_uniform_1i("u_has_texture", true);
            static_shader.set_uniform_1i("u_has_vertex_colors", false);

            static_shader.set_uniform_3f("u_view_pos", m_camera.get_position());
            // static_shader.set_uniform_3f("u_light_pos", m_light_pos);
            // static_shader.set_uniform_3f("u_light_color", m_light_colour);
            static_shader.set_uniform_1i("u_num_lights", m_light_positions.size());
            static_shader.set_uniform_3f_array("u_light_positions", *m_light_positions.data(), m_light_positions.size());
            static_shader.set_uniform_1f_array("u_light_strengths", *m_light_brightnesses.data(), m_light_brightnesses.size());
            static_shader.set_uniform_3f_array("u_light_colours", *m_light_colours.data(), m_light_colours.size());

            if (reg.near_cameras.size() > 0) {
                m_to_be_updated_and_drawn.assign(reg.near_cameras.size(), -1);
                int i = 0;
                for (const auto& entity : reg.near_cameras.entities) {
                    if (
                        entity.get_id() == reg.player.get_id() ||
                        (reg.locomotion_stats.has(entity) && reg.motions.has(entity))
                    ) {
                        m_to_be_updated_and_drawn[i++] = entity.get_id();
                    }
                }
            } else {
                m_to_be_updated_and_drawn.clear();
                m_to_be_updated_and_drawn.push_back(-1);
            }
            _update_models();

            m_renderer->begin_draw();

            _draw_map_and_skybox();
            _draw_walls();
            _draw_health_bars();
            _draw_projectiles();
            _draw_light_orbs();

            for (auto& id : m_to_be_updated_and_drawn) {
                auto kv = m_models.find(id);
                if (kv == m_models.end() || kv->second == nullptr) { continue; }
                kv->second->draw();
            }

            _draw_hud();

            m_renderer->end_draw();

            if (Globals::is_3d_mode) {
                // m_renderer->lock_cursor();
            } else { // Hacky way to quit game.
                // m_renderer->terminate();
                return;
            }

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
        Registry& reg = MapManager::get_instance().get_active_registry();

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
                            if (reg.attackers.get(attacker_entity).weapon == textured_entity.get_id()) {
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

    void _or_something() { 
        std::cout << "Mentally cheeeecked out" << std::endl;
    }

    void unpause() { 
        Globals::in_pause = false; 
        if (!m_game_in_session) {
            m_game_in_session = true;
            _add_resume_to_menu();
        }
    }
    
    void load() { 
        std::cout << "Mentally cheeeecked out" << std::endl;
        if (!m_game_in_session) {
            m_game_in_session = true;
            _add_resume_to_menu();
        }
    }

    void quit() { 
        m_renderer->terminate();
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
           -0.5f,  0.5f, -0.5f,         0.0f, 0.0f, -1.0f,        1, 0, 0, 0,      0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,         0.0f, 0.0f, -1.0f,        1, 0, 0, 0,      1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,         0.0f, 0.0f, -1.0f,        1, 0, 0, 0,      1.0f, 0.0f,
           -0.5f, -0.5f, -0.5f,         0.0f, 0.0f, -1.0f,        1, 0, 0, 0,      0.0f, 0.0f,

            // Back face (Z = 0.5)
           -0.5f,  0.5f,  0.5f,         0.0f, 0.0f, 1.0f,        1, 0, 0, 0,        1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,         0.0f, 0.0f, 1.0f,        1, 0, 0, 0,        0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,         0.0f, 0.0f, 1.0f,        1, 0, 0, 0,        0.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,         0.0f, 0.0f, 1.0f,        1, 0, 0, 0,        1.0f, 0.0f,

            // Left face (X = -0.5)
           -0.5f,  0.5f,  0.5f,         -1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        0.0f, 1.0f,
           -0.5f,  0.5f, -0.5f,         -1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        1.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,         -1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        1.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,         -1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        0.0f, 0.0f,

            // Right face (X = 0.5)
            0.5f,  0.5f,  0.5f,         1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,         1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,         1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,         1.0f, 0.0f, 0.0f,        1, 0, 0, 0,        1.0f, 0.0f,

            // Top face (Y = 0.5)
           -0.5f,  0.5f,  0.5f,         0.0f, 1.0f, 0.0f,        1, 0, 0, 0,        0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,         0.0f, 1.0f, 0.0f,        1, 0, 0, 0,        1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,         0.0f, 1.0f, 0.0f,        1, 0, 0, 0,        1.0f, 0.0f,
           -0.5f,  0.5f, -0.5f,         0.0f, 1.0f, 0.0f,        1, 0, 0, 0,        0.0f, 0.0f,

            // Bottom face (Y = -0.5)
           -0.5f, -0.5f,  0.5f,         0.0f, -1.0f, 0.0f,        1, 0, 0, 0,        0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,         0.0f, -1.0f, 0.0f,        1, 0, 0, 0,        1.0f, 0.0f,
            0.5f, -0.5f, -0.5f,         0.0f, -1.0f, 0.0f,        1, 0, 0, 0,        1.0f, 1.0f,
           -0.5f, -0.5f, -0.5f,         0.0f, -1.0f, 0.0f,        1, 0, 0, 0,        0.0f, 1.0f
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

    void _update_theme() {
        const auto& map_manager = MapManager::get_instance();
        delete m_skybox_texture;
        delete m_wall_texture;
        delete m_map_texture;

        m_skybox_texture = new SkyboxTexture(map_manager.sky_texture_name);
        m_wall_texture = new Texture2D(map_manager.floor_texture_name);
        m_map_texture = new Texture2D(map_manager.floor_texture_name);
    }

    void _draw_light_orbs() {
        // Always skip the camera light :/
        m_light_orb->set_rotation_z(m_light_orb->get_rotation_z() + 0.05);
        for (unsigned int i = 0; i < m_light_positions.size(); ++i) {
            if (m_light_models[i] == nullptr) { continue; }
            m_light_models[i]->set_position(m_light_positions[i]);
            m_light_models[i]->set_position_z(m_light_positions[i].z + 2);
            m_light_models[i]->draw();
        }
    }

    void _draw_map_and_skybox() {
        // Render skybox.
        float size = Common::max_of(MAP_WIDTH, MAP_HEIGHT);
        size += 500;
        GL_Call(glDepthFunc(GL_LEQUAL));
        m_skybox_shader->set_uniform_1i("u_skybox", m_skybox_texture->bind(31));
        m_skybox_shader->set_uniform_mat4f(
            "u_view_project",
            m_camera.get_view_project_matrix()
            * Transform::create_scaling_matrix(glm::vec3(size))
        );
        m_renderer->draw(m_cube_mesh, *m_skybox_shader);
        GL_Call(glDepthFunc(GL_LESS));

        m_floor_shader->set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
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

        m_floor_shader->set_uniform_3f("u_view_pos", m_camera.get_position());
        m_floor_shader->set_uniform_1i("u_num_lights", m_light_positions.size());
        m_floor_shader->set_uniform_3f_array("u_light_positions", *m_light_positions.data(), m_light_positions.size());
        m_floor_shader->set_uniform_1f_array("u_light_strengths", *m_light_brightnesses.data(), m_light_brightnesses.size());
        m_floor_shader->set_uniform_3f_array("u_light_colours", *m_light_colours.data(), m_light_colours.size());

        m_renderer->draw(m_square_mesh, *m_floor_shader);
    }

    void _draw_walls() {
        m_wall_shader->set_uniform_mat4f("u_view_project", m_camera.get_view_project_matrix());
        m_wall_shader->set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });
        m_wall_shader->set_uniform_1i("u_use_repeating_pattern", true);
        m_wall_shader->set_uniform_1i("u_has_texture", true);
        m_wall_shader->set_uniform_1i("u_has_vertex_colors", false);
        m_wall_shader->set_uniform_1i("u_texture", m_wall_texture->bind(29));
        m_wall_shader->set_uniform_mat4f(
            "u_view_project",
            m_camera.get_view_project_matrix()
        );

        m_wall_shader->set_uniform_3f("u_view_pos", m_camera.get_position());
        m_wall_shader->set_uniform_1i("u_num_lights", m_light_positions.size());
        m_wall_shader->set_uniform_3f_array("u_light_positions", *m_light_positions.data(), m_light_positions.size());
        m_wall_shader->set_uniform_1f_array("u_light_strengths", *m_light_brightnesses.data(), m_light_brightnesses.size());
        m_wall_shader->set_uniform_3f_array("u_light_colours", *m_light_colours.data(), m_light_colours.size());

        auto& reg = MapManager::get_instance().get_active_registry();
        for (auto& entity : reg.walls.entities) {
            if (!reg.motions.has(entity)) { continue; }
            auto& motion = reg.motions.get(entity);
            if (glm::distance(motion.position, glm::vec2(m_camera.get_position())) > Globals::static_render_distance) { continue; }
            glm::vec3 wall_scale = glm::vec3(motion.scale, 10.0f);
            m_wall_shader->set_uniform_3f("u_scale", {wall_scale.x / 8, wall_scale.z / 8, wall_scale.y});
            m_wall_shader->set_uniform_mat4f(
                "u_model",
                Transform::create_model_matrix(
                    glm::vec3(motion.position, wall_scale.z / 2),
                    { 0, 0, motion.angle },
                    wall_scale
                )
            );
            m_renderer->draw(m_cube_mesh, *m_wall_shader);
        }

        for (auto& entity : reg.static_objects.entities) {
            if (!reg.motions.has(entity)) { continue; }
            auto& motion = reg.motions.get(entity);
            if (glm::distance(motion.position, glm::vec2(m_camera.get_position())) > Globals::static_render_distance) { continue; }
            auto& static_object = reg.static_objects.get(entity);
            if (static_object.type == STATIC_OBJECT_TYPE::TREE) {
                m_spooky_tree->set_position(glm::vec3(motion.position, -0.3f));
                m_spooky_tree->set_rotation_z(motion.angle);
                m_spooky_tree->draw();
            } else if (static_object.type == STATIC_OBJECT_TYPE::ROCK) {
                m_rocks[1]->set_position(glm::vec3(motion.position, 0.0f));
                m_rocks[1]->set_rotation_z(motion.angle);
                m_rocks[1]->draw();
            } else if (static_object.type == STATIC_OBJECT_TYPE::BONFIRE) {
                m_campfire->set_position(glm::vec3(motion.position, 0.0f));
                m_campfire->set_rotation_z(motion.angle);
                m_campfire->draw();
            } else if (static_object.type == STATIC_OBJECT_TYPE::PORTAL) {
                m_wall_shader->set_uniform_3f("u_object_color", { 1.0f, 1.0f, 1.0f });
                m_wall_shader->set_uniform_3f("u_object_color", { 1.0f, 1.0f, 1.0f });
                m_portal->set_position(glm::vec3(motion.position, 0.0f));
                m_portal->set_rotation_z(motion.angle);
                m_portal->draw();
                // change colour back lol
                m_wall_shader->set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });
            } else if (static_object.type == STATIC_OBJECT_TYPE::DUNGEON_ENTRANCE) {
                m_wall_shader->set_uniform_3f("u_object_color", { 86.0f/ 255.0f, 86.0f / 255.0f, 86.0f / 255.0f });
                m_dungeon_entrance->set_position(glm::vec3(motion.position, 0.0f));
                m_dungeon_entrance->set_rotation_z(motion.angle);
                m_dungeon_entrance->draw();
                // change colour back lol
                m_wall_shader->set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });
            }
        }

        // int x = 0;
        // for (auto& rock : m_rocks) {
        //     rock->set_position_x(x);
        //     rock->draw();
        //     x += 50;
        // }
    }

    void _draw_projectiles() {
        auto& reg = MapManager::get_instance().get_active_registry();

        for (const auto& entity : reg.projectiles.entities) {
            if (!reg.motions.has(entity)) { continue; }
            const auto& projectile = reg.projectiles.get(entity);
            const auto& motion = reg.motions.get(entity);

            // if ()

            if (projectile.projectile_type == PROJECTILE_TYPE::ARROW) {
                m_arrow->set_position(glm::vec3(motion.position, 2.0f));
                m_arrow->set_rotation_z(motion.angle);
                m_arrow->set_rotation_z(motion.angle);
                m_arrow->set_rotation_x(m_arrow->get_rotation_x() + PI / 8);
                m_arrow->draw();
            } else {
                m_banana->set_position(glm::vec3(motion.position, 2.0f));
                m_banana->set_rotation_z(motion.angle);
                m_banana->set_rotation_z(motion.angle);
                m_banana->draw();
            }
        }

    }

    void _draw_health_bars() {
        // Render Health Bars
        auto& reg = MapManager::get_instance().get_active_registry();
        auto& player_motion = reg.motions.get(reg.player);

        for (const auto& loco_entity : reg.locomotion_stats.entities) {
            // Render health bar
            if (loco_entity.get_id() == reg.player.get_id()) { continue; }
            if (!reg.motions.has(loco_entity)) { continue; }
            const auto& loco = reg.locomotion_stats.get(loco_entity);
            const auto& loco_motion = reg.motions.get(loco_entity);
            if (glm::length(player_motion.position - loco_motion.position) > 30.0f) { continue; }
#define HEALTH_BAR_HEIGHT 0.125f

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
            glm::vec3 health_bar_pos;
            glm::vec3 health_bar_angle;

            health_bar_pos = glm::vec3({ loco_motion.position.x, loco_motion.position.y, loco_motion.scale.y + HEALTH_BAR_HEIGHT / 2 + 0.5});

            health_bar_angle = m_camera.get_rotation();

            m_health_shader->set_uniform_mat4f(
                "u_mvp",
                m_camera.get_view_project_matrix()
                * Transform::create_model_matrix(
                    health_bar_pos,
                    health_bar_angle,
                    glm::vec3({ loco_motion.scale.x, HEALTH_BAR_HEIGHT, 1 })
                )
            );
            m_health_shader->set_uniform_3f("u_colour", { 0.05, 0.05, 0.05 });
            m_renderer->draw(m_square_mesh, *m_health_shader);

            // Green health bar layer
            m_health_shader->set_uniform_mat4f(
                "u_mvp",
                m_camera.get_view_project_matrix()
                * Transform::create_model_matrix(
                    health_bar_pos - (0.001f * m_camera.rotate_to_camera_direction({ 0, 0, -1 })),
                    health_bar_angle,
                    glm::vec3({ loco_motion.scale.x * health_percentage, HEALTH_BAR_HEIGHT, 1 })
                )
            );
            m_health_shader->set_uniform_3f("u_colour", { 0.5, 0, 0 });
            m_renderer->draw(m_square_mesh, *m_health_shader);
        }
    }

    void _draw_aim() {
        auto& reg = MapManager::get_instance().get_active_registry();
        if (!reg.locked_target.is_active || !reg.motions.has(reg.locked_target.target)) {return;}
        const auto& motion = reg.motions.get(reg.locked_target.target);
        float cam_angle = _vector_to_angle(motion.position - glm::vec2(m_camera.get_position()));

        const float aspect_ratio = float(m_renderer->get_window_width()) / float(m_renderer->get_window_height());
        const float size = 1.5f;
        const float width = size;
        const float height = size;

        glm::vec3 crosshair_pos = { motion.position.x, motion.position.y, 2.0f }; // Placed on the target position

        m_hud_health_shader->set_uniform_3f("u_colour", glm::vec3(1.0f));
        m_hud_health_shader->set_uniform_1i("u_texture", m_lock_on_reticle->bind(28));
        m_hud_health_shader->set_uniform_1f("u_health_percentage", 1.0f);
        m_hud_health_shader->set_uniform_mat4f("u_model",
            m_camera.get_view_project_matrix() * Transform::create_model_matrix(
                crosshair_pos,
                {PI / 2.0f, 0, cam_angle - PI / 2.0f},
                {width, height, 1}
            )
        );
        m_renderer->draw(m_square_mesh, *m_hud_health_shader);
    }

    void _draw_resource(const glm::vec2& pos, const float size, const float& percent_remaining, const glm::vec3 colour) {
        const float aspect_ratio = float(m_renderer->get_window_width()) / float(m_renderer->get_window_height());

        const float width = size;
        const float height = size * aspect_ratio;

        const float pos_x = pos.x;
        const float pos_y = pos.y;
        const auto& health_bar_container_tranform = Transform::create_model_matrix(
            {pos_x, pos_y, 0.0f},
            {0, 0, 0},
            {width, height, 1}
        );

        m_hud_health_shader->set_uniform_3f("u_colour", colour);
        m_hud_health_shader->set_uniform_1i("u_texture", m_hud_health_texture_fill->bind(28));
        m_hud_health_shader->set_uniform_1f("u_health_percentage", percent_remaining);
        m_hud_health_shader->set_uniform_mat4f("u_model",
            Transform::create_model_matrix(
                {pos_x, pos_y - height * (1.0f - percent_remaining) / 2.0f, 0.0f},
                {0, 0, 0},
                {width, height * percent_remaining, 1}
            )
        );
        m_renderer->draw(m_square_mesh, *m_hud_health_shader);

        m_hud_health_shader->set_uniform_3f("u_colour", glm::vec3(1, 1, 1));
        m_hud_health_shader->set_uniform_mat4f("u_model", health_bar_container_tranform);
        m_hud_health_shader->set_uniform_1i("u_texture", m_hud_health_texture_border->bind(28));
        m_hud_health_shader->set_uniform_1f("u_health_percentage", 1.0f);
        m_renderer->draw(m_square_mesh, *m_hud_health_shader);
    }

    void _draw_loading_screen() {
        m_renderer->begin_draw();
        m_renderer->disable_depth_test();

        const float aspect_ratio = float(m_renderer->get_window_width()) / float(m_renderer->get_window_height());
        const float width = 2;
        const float height = 2;

        const float pos_x = 0.0f;
        const float pos_y = 0.0f;

        m_hud_health_shader->set_uniform_3f("u_colour", glm::vec3(1.0f));
        m_hud_health_shader->set_uniform_1i("u_texture", m_loading_tex->bind(28));
        m_hud_health_shader->set_uniform_1f("u_health_percentage", 1);
        m_hud_health_shader->set_uniform_mat4f("u_model",
            Transform::create_model_matrix(
                {pos_x, pos_y, 0.0f},
                {0, 0, 0},
                {width, height, 1}
            )
        );
        m_renderer->draw(m_square_mesh, *m_hud_health_shader);

        FontStuff& font_stuff = FontStuff::get_instance();
        font_stuff.render_text("Loading...", m_renderer->get_window_width() / 2.0f - m_renderer->get_window_width() / 8.0f, m_renderer->get_window_height() / 12.0f, float(m_renderer->get_window_width()) / (1920.f / 3.f), {1, 1, 1});

        m_renderer->enable_depth_test();
        m_renderer->end_draw();
    }

    std::vector<Texture2D*> m_tutorial_slides;

    void _load_tutorial() {
        if (m_tutorial_slides.size() < 1) {
            m_tutorial_slides.push_back(new Texture2D("tutorial/wasd.png"));
            m_tutorial_slides.push_back(new Texture2D("tutorial/mousemove.png"));
            m_tutorial_slides.push_back(new Texture2D("tutorial/spacebar.png"));
            m_tutorial_slides.push_back(new Texture2D("tutorial/leftclick.png"));
        }
    }

    void _draw_tutorial() {
        _load_tutorial();
        auto& tutorial = TutorialSystem::get_instance();
        const unsigned int state = (unsigned int)tutorial.state;
        if (tutorial.state == TutorialSystem::TUTORIAL_STATE::TUTORIAL_DONE) { return; }
        const float aspect_ratio = float(m_renderer->get_window_width()) / float(m_renderer->get_window_height());
        const float image_aspect_ratio = float(m_tutorial_slides[state]->get_width()) / float(m_tutorial_slides[state]->get_height());
        const float width = 0.5;
        const float height = width / image_aspect_ratio * aspect_ratio;

        m_hud_health_shader->set_uniform_3f("u_colour", {1, 1, 1});
        m_hud_health_shader->set_uniform_1i("u_texture", m_tutorial_slides[state]->bind(27));
        m_hud_health_shader->set_uniform_1f("u_health_percentage", 1.0f);
        m_hud_health_shader->set_uniform_mat4f("u_model",
            Transform::create_model_matrix(
                {1.5f * width / 2, 0, 0},
                {0, 0, 0},
                {width, height, 1}
            )
        );
        m_renderer->draw(m_square_mesh, *m_hud_health_shader);
    }

    void _draw_hud() {
        auto& reg = MapManager::get_instance().get_active_registry();
        auto& player_loco = reg.locomotion_stats.get(reg.player);
        float health_percentage = player_loco.health / player_loco.max_health;
        float energy_percentage = player_loco.energy / player_loco.max_energy;
        float size = 0.25;

        m_renderer->disable_depth_test();
        _draw_aim();

        _draw_resource(
            {-1 + 3 * size / 2, -1 + 3 * size / 2},
            size,
            health_percentage,
            {0.33, 0, 0}
        );

        _draw_resource(
            {1 - 3 * size / 2, -1 + 3 * size / 2},
            size,
            energy_percentage,
            {0, 0.33, 0}
        );

        reg.inventory.estus.size();
        m_hud_health_shader->set_uniform_3f("u_colour", glm::vec3(1.0f));
        m_hud_health_shader->set_uniform_1i("u_texture", m_redbull->bind(28));
        m_hud_health_shader->set_uniform_1f("u_health_percentage", 1.0f);
        float i = 0;
        for (const auto& estus_shit : reg.inventory.estus) {
            m_hud_health_shader->set_uniform_mat4f("u_model",
                Transform::create_model_matrix(
                    {-1 + 1 * size / 2, -1 + i++ * 0.275f + 3 * size / 2, 0.0f},
                    {0, 0, 0},
                    {0.125f, 0.25f, 1}
                )
            );
            m_renderer->draw(m_square_mesh, *m_hud_health_shader);
        }

        _draw_tutorial();

        if (reg.near_interactable.is_active) {
            FontStuff& font_monkey = FontStuff::get_instance();
            font_monkey.render_text(reg.near_interactable.message.c_str(), m_renderer->get_window_width() / 2.0f, m_renderer->get_window_height() / 2.0f, float(m_renderer->get_window_width()) / 1920.f, {0.95f, 0, 0});
        }

        glm::vec3 fps_colour;
        if (m_frame_rate >= 50.0f) {
            fps_colour = {0, 1, 0};
        } else if (m_frame_rate >= 30.0f) {
            fps_colour = {1, 1, 0};
        } else {
            fps_colour = {1, 0, 0};
        }
        FontStuff::get_instance().render_text("fps: " + std::to_string(int(m_frame_rate)), m_renderer->get_window_width() - m_renderer->get_window_width() / 20.0f, m_renderer->get_window_height() - m_renderer->get_window_height() / 20.0f, float(m_renderer->get_window_width()) / (1920.f * 3.0f), fps_colour);

        m_renderer->enable_depth_test();


        // health_bar_pos = glm::vec3(m_camera.get_position());
        // const auto cam_dir_3d = glm::normalize(m_camera.rotate_to_camera_direction({ 0, 0, -1 }));
        // const auto& temp = Transform::create_rotation_matrix({ m_camera.get_rotation().x - PI / 2, m_camera.get_rotation().y, m_camera.get_rotation().z }) * glm::vec4(0, 0, -1, 1);
        // const auto& down_from_camera = glm::vec3(temp);
        // const auto& left_from_camera = glm::normalize(glm::cross(cam_dir_3d, down_from_camera));
        // health_bar_pos += (-2.75f * down_from_camera) + (3.0f * cam_dir_3d) + (2.75f * left_from_camera);
        // health_bar_pos += glm::vec3();

    }

    void _update_models() {
        auto& reg = MapManager::get_instance().get_active_registry();
        for (auto& id : m_to_be_updated_and_drawn) {
            if (id < 0) { continue; }
            auto& model = m_models[id];
            if (model == nullptr) {
                continue;
            }
            Entity entity = reinterpret_cast<Entity&>(id);
            if (!reg.motions.has(entity)) {
                delete model;
                m_models[id] = nullptr;
                continue;
            }
            auto& motion = reg.motions.get(id);

            auto angle = std::fmod(motion.angle, 2 * PI);
            if (angle < 0) {
                angle += 2 * PI;
            }
            auto velocity_angle = _vector_to_angle(motion.velocity);
            auto dot_between_view_velo_directions = glm::dot(
                glm::normalize(
                    glm::vec2(Transform::create_rotation_matrix({0, 0, angle}) * glm::vec4(1, 0, 0, 0))
                ),
                glm::normalize(motion.velocity)
            );
            dot_between_view_velo_directions = glm::clamp(dot_between_view_velo_directions, -0.9999999f, 0.9999999f);
            float angle_between_view_and_velo = std::acos(dot_between_view_velo_directions);

            // bool is_dodging = false;
            bool rotate_to_velocity_dir = false;
            bool rotate_opposite_to_velocity_dir = false;
            bool is_zombie = model->get_name() == "Zombie Grunt.dae";
            // const float buffer_time = 0.45f;
            // const float buffer_time = 0.25f;
            const float buffer_time = 0.5f;
            if (reg.death_cooldowns.has(entity)) {
                model->force_play_animation("Dying.dae", -1, false, true);
            } else if (reg.in_rests.has(entity) && reg.player.get_id() == entity.get_id()) {
                if (!m_player_was_in_rest) {
                    model->force_play_animation("Stand Up.dae", -1, false, true, true);
                    m_player_was_in_rest = true;
                }
            } else if (
                Globals::is_getting_up &&
                model->get_current_animation_id() == model->get_animation_id("Stand Up.dae") &&
                model->get_portion_complete_of_curr_animation() > 0.95f
            ) {
                if (m_player_was_in_rest) {
                    model->force_play_animation("Sitting.dae");
                    model->force_play_animation("Stand Up.dae", -1, false, true);
                    m_player_was_in_rest = false;
                } else {
                    Globals::is_getting_up = false;
                }
            } else if (reg.stagger_cooldowns.has(entity)) {
                const auto& cooldown = reg.stagger_cooldowns.get(entity);
                model->force_play_animation("Stagger.dae", cooldown.timer + buffer_time);
            } else if (reg.death_cooldowns.has(reg.player)) {
                const auto& cooldown = reg.death_cooldowns.get(reg.player);
                model->force_play_animation("Dance.dae", cooldown.timer + 2.0f * buffer_time);
            } else {
                if (reg.in_dodges.has(entity)) {
                    const auto& dodge = reg.in_dodges.get(entity);
                    model->force_play_animation("Roll.dae", dodge.duration + buffer_time, false, true);
                }

                if (reg.attack_cooldowns.has(entity)) {
                    const auto& cooldown = reg.attack_cooldowns.get(entity);
                    if (glm::length(motion.velocity) > 0.0f) {
                        model->play_animation("Running Attack.dae", cooldown.timer + buffer_time, false, true);
                    } else {
                        model->play_animation("Standing Attack.dae", cooldown.timer + buffer_time, false, true);
                    }
                }

                if (glm::length(motion.velocity) > 0.0f) {
                    // const auto& speed = glm::length(motion.velocity);
                    if (angle_between_view_and_velo < PI / 3 || is_zombie) {
                        model->play_animation("Forward.dae", 0.7f);
                    } else if (angle_between_view_and_velo > 2 * PI / 3) {
                        model->play_animation("Backward.dae", 0.7f);
                    } else if (
                        (velocity_angle - angle < PI && velocity_angle - angle >= 0)
                        || velocity_angle - angle < -PI && velocity_angle - angle < 0) {
                        model->play_animation("Left.dae", 0.75f);
                    } else {
                        model->play_animation("Right.dae", 0.75f);
                    }

                } else {
                    model->play_animation("default0");
                }
            }


            const auto current_anim_id = model->get_current_animation_id();
            if (glm::length(motion.velocity) > 0.001f) {

                if (current_anim_id == model->get_animation_id("Roll.dae")) {
                    rotate_to_velocity_dir = true;
                } else if (current_anim_id == model->get_animation_id("Running Attack.dae")) {
                    // rotate_to_velocity_dir = true;
                } else if (current_anim_id == model->get_animation_id("Forward.dae")) {
                    rotate_to_velocity_dir = true;
                } else if (current_anim_id == model->get_animation_id("Backward.dae")) {
                    rotate_opposite_to_velocity_dir = true;
                }

            }
            model->set_position(glm::vec3(motion.position, 0.0f));

            if (rotate_to_velocity_dir) {
                model->set_rotation_z(velocity_angle);
            } else if (rotate_opposite_to_velocity_dir) {
                model->set_rotation_z(velocity_angle - PI);
            } else {
                model->set_rotation_z(motion.angle);
            }

            model->update();
        }
    }

    void _add_resume_to_menu() {
        main_menu->add_element(
            std::make_unique<Button>(
                &m_square_mesh,
                glm::vec2(0, 0.1f * 1.5f * 2.0f),
                glm::vec2(0.25f, 0.1f),
                "",
                m_menu_textures["resume"],
                m_menu_textures["hover_resume"],
                std::bind(&Application::unpause, this)
            )
        );
    }

    void _init_menu() {
        main_menu = std::make_unique<Menu>("menu/home.jpg");
        main_menu->add_element(
            std::make_unique<Button>(
                &m_square_mesh,
                glm::vec2(0, 0.1f * 1.5f * 1.0f),
                glm::vec2(0.25f, 0.1f),
                "",
                m_menu_textures["play"],
                m_menu_textures["hover_play"],
                std::bind(&Application::unpause, this)
            )
        );
        main_menu->add_element(
            std::make_unique<Button>(
                &m_square_mesh,
                glm::vec2(0, 0.1f * 1.5f * 0.0f),
                glm::vec2(0.25f, 0.1f),
                "",
                m_menu_textures["load"],
                m_menu_textures["hover_load"],
                std::bind(&Application::load, this)
            )
        );
        main_menu->add_element(
            std::make_unique<Button>(
                &m_square_mesh,
                glm::vec2(0, 0.1f * 1.5f * -1.0f),
                glm::vec2(0.25f, 0.1f),
                "",
                m_menu_textures["quit"],
                m_menu_textures["hover_quit"],
                std::bind(&Application::quit, this)
            )
        );
    }

    void menu_update() {
        if (glfwGetMouseButton((GLFWwindow *)m_renderer->get_window(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double mouse_x, mouse_y;
            glfwGetCursorPos((GLFWwindow *)m_renderer->get_window(), &mouse_x, &mouse_y);

            main_menu->handle_click(float(mouse_x), float(mouse_y));
        }

        double mouse_x, mouse_y;
        // Get mouse position from your window system
        glfwGetCursorPos((GLFWwindow *)m_renderer->get_window(), &mouse_x, &mouse_y);
        // float x, y;
        // x = float(mouse_x);
        // y = float(mouse_y);
        main_menu->update(
            2.0f * (float(mouse_x) / float(m_renderer->get_window_width()) - 0.5f), 
            -2.0f * (float(mouse_y) / float(m_renderer->get_window_height()) - 0.5f));
        // main_menu->update(x, y);
        // main_menu->update(100, 200);

        // m_renderer->
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