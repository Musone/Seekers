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
#include <renderer/AnimatedModel.hpp>
#include <renderer/StaticModel.hpp>
#include <ecs/Registry.hpp>
#include <app/World.h>
#include <app/InputManager.hpp>
#include <app/TextureMaster.hpp>
#include <components/RenderComponents.hpp>
#include <components/CombatComponents.hpp>

#include <cstdint>
#include <queue>

#include <glm/glm.hpp>

#include <globals/Globals.h>

#include <utils/CalladaTokenizer.hpp>

#include <iomanip>

#include <assimp/vector3.h>

namespace Testing {
    glm::vec3 child_position(0.0f);
    AnimatedModel::Attachment* attach;

    void _handle_free_camera_inputs(const Renderer& renderer, Camera& cam) {
        glm::vec3 moveDirection(0.0f);
        glm::vec3 rotateDirection(0.0f);

        // float moveSpeed = 0.05f;
        float moveSpeed = 3.5f;
        float rotateSpeed = PI / 75;  // radians per frame
        // float rotateSpeed = PI / 15;  // radians per frame

        glm::vec3 newPosition = 1.f * cam.get_position();
        glm::vec3 newRotation = 1.f * cam.get_rotation();
        bool pos_changed = false;
        bool rot_changed = false;

        glm::vec3 player_input(0.0f);

        if (renderer.is_key_pressed(GLFW_KEY_P)) {
            std::cout << "X: " << std::to_string(child_position.x) << " | Y: " << std::to_string(child_position.y) << " | Z: " << std::to_string(child_position.z) << '\n';
        }

        if (renderer.is_key_pressed(GLFW_KEY_EQUAL)) {
            attach->offset_scale += moveSpeed;
        }
        if (renderer.is_key_pressed(GLFW_KEY_MINUS)) {
            attach->offset_scale -= moveSpeed;
        }
        

        if (renderer.is_key_pressed(GLFW_KEY_B)) {
            attach->offset_rotation.x += rotateSpeed;
        }
        if (renderer.is_key_pressed(GLFW_KEY_N)) {
            attach->offset_rotation.y += rotateSpeed;
        }
        if (renderer.is_key_pressed(GLFW_KEY_M)) {
            attach->offset_rotation.z += rotateSpeed;
        }

        if (renderer.is_key_pressed(GLFW_KEY_I)) {
            child_position.y += moveSpeed;
            attach->offset_position = child_position;
        }
        if (renderer.is_key_pressed(GLFW_KEY_K)) {
            child_position.y -= moveSpeed;
            attach->offset_position = child_position;
        }
        if (renderer.is_key_pressed(GLFW_KEY_J)) {
            child_position.x -= moveSpeed;
            attach->offset_position = child_position;
        }
        if (renderer.is_key_pressed(GLFW_KEY_L)) {
            child_position.x += moveSpeed;
            attach->offset_position = child_position;
        }
        if (renderer.is_key_pressed(GLFW_KEY_COMMA)) {
            child_position.z += moveSpeed;
            attach->offset_position = child_position;
        }
        if (renderer.is_key_pressed(GLFW_KEY_PERIOD)) {
            child_position.z -= moveSpeed;
            attach->offset_position = child_position;
        }

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

    void _print_matrices(const std::vector<glm::mat4>& matrices) {
        for (size_t i = 0; i < matrices.size(); ++i) {
            std::cout << "\nMatrix " << i << ":\n";
            const glm::mat4& mat = matrices[i];
            for (int row = 0; row < 4; ++row) {
                std::cout << "| ";
                for (int col = 0; col < 4; ++col) {
                    // Access element and format with fixed precision
                    std::cout << std::fixed << std::setprecision(6) << std::setw(10) 
                            << mat[col][row] << " ";
                }
                std::cout << "|\n";
            }
            std::cout << std::endl;
        }
    }

    void try_assimp() {
        Renderer& renderer = Renderer::get_instance();
        renderer.init(
            "3d animated model assimp Demo",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            true,
            false
        );
        
        Shader animated_shader("AnimatedBlinnPhong");
        Shader static_shader("StaticBlinnPhong");
        AnimatedModel hero("models/Hero/Hero.dae", &animated_shader);
        hero.set_rotation(PI / 2, 0, 0);

        StaticModel tree("models/Lowpoly_tree_sample.dae", &static_shader);
        tree.m_has_texture = false;
        tree.set_scale(20, 20, 20);
        tree.set_rotation(PI / 2, 0, 0);
        tree.set_position(500, 500, 0);
        // StaticModel sword("models/sword.dae", &static_shader);
        StaticModel bow("models/Sword.obj", &static_shader);
        bow.set_scale(10, 10, 10);
        // bow.set_position(750, 0, 0);
        StaticModel katana("models/katana.obj", &static_shader);
        // StaticModel katana("models/Halo Energy Sword.dae", &static_shader);
        // StaticModel bow("models/bow.obj", m_wall_shader);
        StaticModel arrow("models/Arrow.dae", &static_shader);
        arrow.set_scale(glm::vec3(300));

        StaticModel melee("models/Melee.obj", &static_shader);
        melee.set_scale(glm::vec3(1));
        
        // Model delete_me("models/Lowpoly_tree_sample.dae");
        // Model delete_me("models/Hero.dae");

        attach = hero.attach_to_joint(
            &bow, 
            "mixamorig_RightHand", 
            child_position, // pos 
            // glm::vec3(- PI / 2, - PI / 2, 0), // rot
            glm::vec3(0, - PI / 2, PI / 2), // rot
            glm::vec3(1, 1, 1) // scale
        );

        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        // cam.set_position({ 0, -200, 100 });
        cam.set_position({ -100, 0, 0 });
        cam.set_rotation({ PI / 2, 0, -PI / 2 });
        // cam.set_rotation({ PI / 2, 0, 0 });
        
        Timer timer;
        float time_of_last_frame = 0;

        // hero.load_animation_from_file("models/dying.dae");
        // if (hero.get_animation_count() > 0) {
        //     hero.play_animation("models/dying.dae");
        // }


        // AnimatedModel guy("models/Sword_and_Shield_Pack/Ch43_nonPBR.dae", &animated_shader);
        // AnimatedModel guy("models/dying.dae", &animated_shader);
        // guy.set_scale(50, 50, 50);
        // guy.set_position(-500, 500, 0);
        // guy.set_rotation(PI / 2, 0, 0);
        // guy.load_animation_from_file("models/dying.dae");
        // guy.load_animation_from_file("models/Sword_and_Shield_Pack/sheath sword 2.dae");
        // guy.load_animation_from_file("models/Sword_and_Shield_Pack/sword and shield 180 turn.dae");
        // if (guy.get_animation_count() > 0) {
        //     guy.play_animation(1);
        // }

        StaticModel* m_spooky_tree = new StaticModel("models/Spooky Tree/Spooky Tree.obj", &static_shader);
        m_spooky_tree->m_has_texture = true;
        m_spooky_tree->texture_list.push_back(std::make_shared<Texture2D>("Spooky Tree.jpg"));
        m_spooky_tree->mesh_list.back()->set_texture(m_spooky_tree->texture_list.back());
        m_spooky_tree->set_pre_transform(
            Transform::create_scaling_matrix(glm::vec3(0.15f, 0.15f, 0.35f))
        );
        m_spooky_tree->set_position({-50, -50, 0});

        AnimatedModel wolf("models/griffin_animated/griffin_animated.gltf", &animated_shader);
        wolf.set_scale(50, 50, 50);
        wolf.set_position(0, 500, 0);
        if (wolf.get_animation_count() > 0) {
            wolf.play_animation(0);
        }

        hero.print_animations();
        // guy.print_animations();
        wolf.print_animations();

        // StaticModel campfire("models/Campfire.stl", &static_shader);
        StaticModel campfire("models/Campfire.obj", &static_shader);
        // StaticModel campfire("models/Campfire.fbx", &static_shader);
        // AnimatedModel campfire("models/Campfire.fbx", &animated_shader);
        // AnimatedModel campfire("models/Campfire_anim.fbx", &animated_shader);
        campfire.m_has_texture = true;
        campfire.texture_list.push_back(std::make_shared<Texture2D>("Campfire_MAT_BaseColor_00.jpg"));
        // campfire.texture_list.push_back(std::make_shared<Texture2D>("Campfire_MAT_BaseColor_01.jpg"));
        campfire.mesh_list.back()->set_texture(campfire.texture_list.back());
        campfire.set_scale(glm::vec3(1));
        campfire.set_position({50, 50, 0}); 
        // campfire.load_animation_from_file("models/Campfire_anim.fbx");
        // campfire.print_animations();
        // campfire.play_animation("default0");
        // campfire.play_animation("Campfire_anim.fbx");


        const float FRAME_TIME_60FPS = 1000000.0f / 60.0f;  // microseconds per frame at 60 FPS
        while (!renderer.is_terminated()) {
            float delta_time = float(timer.GetTime()) - time_of_last_frame;
            while (delta_time < FRAME_TIME_60FPS) {
                delta_time = float(timer.GetTime()) - time_of_last_frame;
            }

            time_of_last_frame = float(timer.GetTime());
            // model.set_scale({20, 20, 20});
            _handle_free_camera_inputs(renderer, cam);
            
            hero.update();
            wolf.update();
            // campfire.update();
            // guy.update();

            renderer.begin_draw();
            glm::vec3 light_pos = cam.get_position();

            animated_shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());
            animated_shader.set_uniform_3f("u_view_pos", cam.get_position());
            animated_shader.set_uniform_3f("u_light_pos", light_pos);
            animated_shader.set_uniform_3f("u_light_color", { 1, 1, 1 });
            animated_shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            static_shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());
            static_shader.set_uniform_3f("u_view_pos", cam.get_position());
            static_shader.set_uniform_3f("u_light_pos", light_pos);
            static_shader.set_uniform_3f("u_light_color", { 1, 1, 1 });
            static_shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            hero.draw();
            tree.draw();
            campfire.draw(); 
            m_spooky_tree->draw();
            // bow.draw();
            // wolf.draw();
            // guy.draw();
            // melee.set_rotation(attach->offset_rotation);
            // melee.draw();
            // arrow.set_rotation(attach->offset_rotation);
            // arrow.set_rotation(attach->offset_rotation);
            // arrow.draw();


            renderer.end_draw();
        }
    }
    
};