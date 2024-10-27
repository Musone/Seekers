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
#include <renderer/Model.hpp>
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
    // glm::vec3 child_position(-80.5f, 24.5f, -14.0f);
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

    void _print_vertex_data(const void* data, size_t vertex_count) {
        const float* float_data = static_cast<const float*>(data);
        const int* int_data = static_cast<const int*>(data);
        
        // Calculate offsets based on your layout
        const size_t floats_before_joints = 8;  // 3 pos + 3 normal + 2 uv
        const size_t ints_per_vertex = 10;      // 5 pairs of joint indices
        const size_t floats_for_weights = 10;   // 5 pairs of weights
        const size_t stride_in_floats = floats_before_joints + (ints_per_vertex * sizeof(int)/sizeof(float)) + floats_for_weights;
        
        for (size_t i = 0; i < vertex_count; i++) {
            std::cout << "\nVertex " << i << ":\n";
            
            // Print position
            size_t base_idx = i * stride_in_floats;
            std::cout << "Position: (" 
                    << float_data[base_idx] << ", "
                    << float_data[base_idx + 1] << ", "
                    << float_data[base_idx + 2] << ")\n";
            
            // Print joint indices (need to cast to int*)
            const int* joint_data = reinterpret_cast<const int*>(&float_data[base_idx + floats_before_joints]);
            std::cout << "Joint Indices:\n";
            for (int j = 0; j < 5; j++) {  // 5 pairs of joints
                std::cout << "  Pair " << j << ": ("
                        << joint_data[j*2] << ", "
                        << joint_data[j*2 + 1] << ")\n";
            }
            
            // Print weights
            size_t weights_start = base_idx + floats_before_joints + (ints_per_vertex * sizeof(int)/sizeof(float));
            std::cout << "Weights:\n";
            for (int j = 0; j < 5; j++) {  // 5 pairs of weights
                std::cout << "  Pair " << j << ": ("
                        << float_data[weights_start + j*2] << ", "
                        << float_data[weights_start + j*2 + 1] << ")\n";
            }
            
            std::cout << "------------------------\n";
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
        // Model hero("objs/Hero.dae");
        AnimatedModel hero("objs/Hero.dae", &animated_shader);
        StaticModel tree("objs/Lowpoly_tree_sample.dae", &static_shader);
        StaticModel katana("objs/katana.obj", &static_shader);
        tree.set_scale(20, 20, 20);
        hero.set_rotation_x(PI / 2);
        
        // Model delete_me("objs/Lowpoly_tree_sample.dae");
        // Model delete_me("objs/Hero.dae");

        attach = hero.attach_to_joint(
            &katana, 
            "mixamorig_RightHand", 
            child_position, // pos 
            // glm::vec3(- PI / 2, - PI / 2, 0), // rot
            glm::vec3(0, - PI / 2, PI / 2), // rot
            glm::vec3(1, 1, 1) // scale
        );

        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        cam.set_position({ 0, -200, 100 });
        cam.set_rotation({ PI / 2, 0, 0 });
        
        Timer timer;
        float time_of_last_frame = 0;

        if (hero.get_animation_count() > 0) {
            hero.play_animation(0);
        }

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

            renderer.end_draw();
        }
    }
    
};