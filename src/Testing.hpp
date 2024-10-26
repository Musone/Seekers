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
#include <ecs/Registry.hpp>
#include <app/World.h>
#include <app/InputManager.hpp>
#include <app/TextureMaster.hpp>
#include <components/RenderComponents.hpp>
#include <components/CombatComponents.hpp>

#include <cstdint>
#include <queue>

#include <globals/Globals.h>

#include <utils/CalladaTokenizer.hpp>

#include <iomanip>

#include <assimp/vector3.h>

class SimpleJoint {
public:
    int id;
    std::string name;
    std::vector<SimpleJoint*> children;

    // DFS with recursion
    void update_joint_transform(glm::mat4* current_pose_matrices, glm::mat4* animated_matrices, const int& parent_id) {
        if (parent_id > -1 && id > -1) {
            animated_matrices[id] = animated_matrices[parent_id] * current_pose_matrices[id];
        } else if (id > -1) {
            animated_matrices[id] = current_pose_matrices[id];
        }

        for (const auto& c : children) {
            c->update_joint_transform(current_pose_matrices, animated_matrices, id);
        }
    }

    // DFS with iteration
    // void update_joint_transform(glm::mat4* current_pose_matrices, glm::mat4* animated_matrices, const int& parent_id) {
    //     struct JointUpdate {
    //         SimpleJoint* joint;
    //         int parent_id;
    //     };

    //     std::stack<JointUpdate> update_stack;
    //     update_stack.push({this, parent_id});

    //     while (!update_stack.empty()) {
    //         auto current = update_stack.top();
    //         update_stack.pop();

    //         SimpleJoint* joint = current.joint;
    //         int current_parent_id = current.parent_id;

    //         // Update current joint's transform
    //         if (current_parent_id > -1 && joint->id > -1) {
    //             animated_matrices[joint->id] = animated_matrices[current_parent_id] * current_pose_matrices[joint->id];
    //         } else if (joint->id > -1) {
    //             animated_matrices[joint->id] = current_pose_matrices[joint->id];
    //         }

    //         // Add children to stack (in reverse order to maintain same processing order as recursive version)
    //         for (auto it = joint->children.rbegin(); it != joint->children.rend(); ++it) {
    //             update_stack.push({*it, joint->id});
    //         }
    //     }
    // }
};

namespace Testing {
    int the_pose_I_wanna_show = 0;
    int counting = 0;
    std::vector<std::string> joint_names;
    glm::mat4* current_pose;
    glm::mat4* local_bind_transforms;
    // glm::mat4 current_pose[16];
    std::vector<Token*> animation_list;
    std::unordered_map<std::string, unsigned int> joint_name_to_id;
    const int total_animation_frames = 2;

    void _handle_free_camera_inputs(const Renderer& renderer, Camera& cam) {
        glm::vec3 moveDirection(0.0f);
        glm::vec3 rotateDirection(0.0f);

        // float moveSpeed = 0.05f;
        float moveSpeed = 2.5f;
        // float rotateSpeed = PI / 300;  // radians per frame
        float rotateSpeed = PI / 15;  // radians per frame

        glm::vec3 newPosition = 1.f * cam.get_position();
        glm::vec3 newRotation = 1.f * cam.get_rotation();
        bool pos_changed = false;
        bool rot_changed = false;

        glm::vec3 player_input(0.0f);

        if (renderer.is_key_pressed(GLFW_KEY_TAB)) {
            // Reset pose
            for (unsigned int i = 0; i < joint_names.size(); i++) {
                current_pose[i] = local_bind_transforms[i];
            }
            the_pose_I_wanna_show = ++counting % total_animation_frames;
            for (const auto& animation : animation_list) {
                Token* channel = animation->get_tokens("channel")[0];
                std::string joint_name = Common::split_string(channel->props["target"], '/')[0];
                
                if (joint_name_to_id.find(joint_name) == joint_name_to_id.end()) {
                    continue;
                }
                
                unsigned int joint_id = joint_name_to_id[joint_name];
                const int base_index = the_pose_I_wanna_show * 16;

                std::vector<Token*> source_list = animation->get_tokens("source");
                std::string raw_keyframes = source_list[1]->get_tokens("float_array")[0]->data;
                std::vector<float> keyframes;
                keyframes.reserve(80);
                for (const auto& s : Common::split_string(raw_keyframes, ' ')) {
                    keyframes.push_back(std::stof(s));
                }

                current_pose[joint_id] = glm::mat4(
                    keyframes[base_index + 0], keyframes[base_index + 4], keyframes[base_index + 8], keyframes[base_index + 12],
                    keyframes[base_index + 1], keyframes[base_index + 5], keyframes[base_index + 9], keyframes[base_index + 13],
                    keyframes[base_index + 2], keyframes[base_index + 6], keyframes[base_index + 10], keyframes[base_index + 14],
                    keyframes[base_index + 3], keyframes[base_index + 7], keyframes[base_index + 11], keyframes[base_index + 15]
                );

                // current_pose[joint_id] = glm::mat4(1.0f);
            }
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
        
        Shader shader("AnimatedBlinnPhong");
        Model hero("objs/Hero.dae");
        hero.set_rotation_x(PI / 2);
        

        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        cam.set_position({ 0, -200, 100 });
        cam.set_rotation({ PI / 2, 0, 0 });
        
        Timer timer;
        float time_of_last_frame = 0;

        while (!renderer.is_terminated()) {
            float delta_time = float(timer.GetTime()) - time_of_last_frame;
            while (delta_time < 125000.0f) {
                delta_time = float(timer.GetTime()) - time_of_last_frame;
            }

            time_of_last_frame = float(timer.GetTime());
            // model.set_scale({20, 20, 20});
            _handle_free_camera_inputs(renderer, cam);
            
            renderer.begin_draw();
            glm::vec3 light_pos = cam.get_position();

            shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());

            shader.set_uniform_3f("u_view_pos", cam.get_position());
            shader.set_uniform_3f("u_light_pos", light_pos);
            shader.set_uniform_3f("u_light_color", { 1, 1, 1 });
            shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            hero.draw(shader);

            renderer.end_draw();
        }
    }

    void do_stuff() {
        // const std::string collada_file_name = "thin_mat_model.dae";
        // const std::string texture_file_name = "thin_mat_model.png";
        // const std::string collada_file_name = "BlackDragon/Dragon.dae";
        // const std::string texture_file_name = "Dragon_ground_color.jpg";
        const std::string collada_file_name = "Wolf_dae.dae";
        const std::string texture_file_name = "Wolf_Body.jpg";

        // std::string xml_content = FileSystem::read_file("objs/BlackDragon/Dragon.dae");
        // std::string xml_content = FileSystem::read_file("objs/thin_mat_model.dae");
        std::string xml_content = FileSystem::read_file("objs/" + collada_file_name);
        
        std::vector<Token> tokens;
        try {
            tokens = tokenize(xml_content);
        } catch (const std::exception& e) {
            const std::string message = std::string(e.what());
            Log::log_error_and_terminate(message, __FILE__, __LINE__);
        }

        Token* collada = &tokens[0];
        Token* library_geometries = collada->get_tokens("library_geometries")[0];
        std::vector<Token*> geometry_list = library_geometries->get_tokens("geometry");

        Token* library_controllers = collada->get_tokens("library_controllers")[0];

#pragma region trying to rig skeleton
        std::vector<Token*> controller_source_list = library_controllers->get_tokens("source");
        std::string raw_joint_names = controller_source_list[0]->get_tokens("Name_array")[0]->data;
        joint_names = Common::split_string(raw_joint_names, ' ');

        std::string raw_weights = controller_source_list[2]->get_tokens("float_array")[0]->data;
        std::vector<float> weights;
        for (const auto& s : Common::split_string(raw_weights, ' ')) {
            weights.push_back(std::stof(s));
        }

        Token* vertex_weights = library_controllers->get_tokens("vertex_weights")[0];
        std::string raw_joint_weight_vcounts = vertex_weights->get_tokens("vcount")[0]->data;
        std::vector<unsigned int> joint_weight_vcounts;
        for (const auto& s : Common::split_string(raw_joint_weight_vcounts, ' ')) {
            joint_weight_vcounts.push_back(std::stoi(s));
        }

        std::string raw_joint_weight_indices = vertex_weights->get_tokens("v")[0]->data;
        std::vector<unsigned int> bad_joint_weight_indices;
        for (const auto& s : Common::split_string(raw_joint_weight_indices, ' ')) {
            bad_joint_weight_indices.push_back(std::stoi(s));
        }

        std::vector<int> joint_weight_indices;
        joint_weight_indices.reserve((10 + 10) * joint_weight_vcounts.size()); // at most 10 joint and 10 weight per vertex.
        unsigned int base_of_joint_weight = 0;
        for (unsigned int vertex_id = 0; vertex_id < joint_weight_vcounts.size(); ++vertex_id) {
            const auto joint_count = joint_weight_vcounts[vertex_id];

            // We need to add padding here so that I can use a regular stride to index joints and weights, 
            // and push them to the VBO.
            for (unsigned int offset = 0; offset < 10; ++offset) { 
                // add joint indices
                if (offset < joint_count) {
                    joint_weight_indices.push_back(bad_joint_weight_indices[base_of_joint_weight + 2 * offset]);
                } else {
                    // add some padding
                    joint_weight_indices.push_back(-1);
                }
            }

            for (unsigned int offset = 0; offset < 10; ++offset) { 
                if (offset < joint_count) {
                    // add weight indices
                    joint_weight_indices.push_back(bad_joint_weight_indices[base_of_joint_weight + 2 * offset + 1]);
                } else {
                    // add some padding
                    joint_weight_indices.push_back(-1);
                }
            }

            base_of_joint_weight += 2 * joint_count;
        }

#pragma endregion

#pragma region building the vertex buffer
        std::vector<unsigned int> indices;
        std::vector<float> vertices;
        unsigned int i_curret_vertex = 0;

        for (const auto& geometry : geometry_list) {
            std::vector<Token*> polylist_list = geometry->get_tokens("polylist");
            for (const auto& polylist : polylist_list) {
                
                std::string raw_indices = polylist->get_tokens("p")[0]->data;
                std::vector<unsigned int> collada_indices;
                for (const auto& s : Common::split_string(raw_indices, ' ')) {
                    collada_indices.push_back(std::stoi(s));
                }
                
                std::string raw_vcount = polylist->get_tokens("vcount")[0]->data;
                std::vector<unsigned int> vcounts;
                for (const auto& s : Common::split_string(raw_vcount, ' ')) {
                    vcounts.push_back(std::stoi(s));
                }

                std::vector<Token*> source_list = geometry->get_tokens("source");
                std::string raw_positions = source_list[0]->get_tokens("float_array")[0]->data;
                std::vector<float> positions;
                for (const auto& s : Common::split_string(raw_positions, ' ')) {
                    positions.push_back(std::stof(s));
                }

                std::string raw_normals = source_list[1]->get_tokens("float_array")[0]->data;
                std::vector<float> normals;
                for (const auto& s : Common::split_string(raw_normals, ' ')) {
                    normals.push_back(std::stof(s));
                }

                std::string raw_texcoords0 = source_list[2]->get_tokens("float_array")[0]->data;
                std::vector<float> texcoords0;
                for (const auto& s : Common::split_string(raw_texcoords0, ' ')) {
                    texcoords0.push_back(std::stof(s));
                }

                // const auto elements_in_an_index_bundle = 4; // pos norm tex colour (thin matrix model)

                auto input_list = polylist->get_tokens("input");
                unsigned int largest_offest = 0;
                for (const auto& input : input_list) {
                    const unsigned int offset = std::stoi(input->props["offset"]);
                    if (offset > largest_offest) {
                        largest_offest = offset;
                    }
                }

                const auto elements_in_an_index_bundle = largest_offest + 1; // pos norm tex colour (thin matrix model)
                for (unsigned int i = 0; i < vcounts.size(); ++i) {
                    auto count = vcounts[i];
                    const auto base_index_bundle = i * elements_in_an_index_bundle * count; // there are 4 vertex. Each has pos and norm.
                    
                    for (auto& offset : { 0, 1, 2 }) {
                        const auto i_pos = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
                        const auto i_norm = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 1];
                        const auto i_tex = 2 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 2];
                        vertices.push_back(positions[i_pos]);     // x
                        vertices.push_back(positions[i_pos + 1]); // y
                        vertices.push_back(positions[i_pos + 2]); // z
                        vertices.push_back(normals[i_norm]);     // x
                        vertices.push_back(normals[i_norm + 1]); // y
                        vertices.push_back(normals[i_norm + 2]); // z
                        vertices.push_back(texcoords0[i_tex]);
                        vertices.push_back(texcoords0[i_tex + 1]);

                        // i_pos is basically the vertex id. Now we should be able to get exactly the joint and weight offset.
                        const auto base_index_joint_weight = 20 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
                        for (unsigned int j = 0; j < 20; ++j) {
                            if (j < 10) { // joint
                                int joint_id = joint_weight_indices[base_index_joint_weight + j];
                                vertices.push_back(reinterpret_cast<float&>(joint_id));
                            } else { // weight
                                const int weight_id = joint_weight_indices[base_index_joint_weight + j];
                                if (weight_id < 0) {
                                    vertices.push_back(0.0f);
                                } else {
                                    const float weight = weights[weight_id];
                                    vertices.push_back(weight);
                                }
                            }
                        }
                        
                        indices.push_back(i_curret_vertex++);
                    }
                    if (count > 3) { // it is a quad
                        for (auto& offset : { 0, 2, 3 }) {
                            const auto i_pos = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
                            const auto i_norm = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 1];
                            const auto i_tex = 2 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 2];
                            vertices.push_back(positions[i_pos]);     // x
                            vertices.push_back(positions[i_pos + 1]); // y
                            vertices.push_back(positions[i_pos + 2]); // z
                            vertices.push_back(normals[i_norm]);     // x
                            vertices.push_back(normals[i_norm + 1]); // y
                            vertices.push_back(normals[i_norm + 2]); // z
                            vertices.push_back(texcoords0[i_tex]);
                            vertices.push_back(texcoords0[i_tex + 1]);

                            // i_pos is basically the vertex id. Now we should be able to get exactly the joint and weight offset.
                            const auto base_index_joint_weight = 20 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
                            for (unsigned int j = 0; j < 20; ++j) {
                                if (j < 10) { // joint
                                    float joint_id = joint_weight_indices[base_index_joint_weight + j];
                                    vertices.push_back(reinterpret_cast<float&>(joint_id));
                                } else { // weight
                                    const int weight_id = joint_weight_indices[base_index_joint_weight + j];
                                    if (weight_id < 0) {
                                        vertices.push_back(0.0f);
                                    } else {
                                        const float weight = weights[weight_id];
                                        vertices.push_back(weight);
                                    }
                                }
                            }

                            indices.push_back(i_curret_vertex++);
                        }
                    }
                }
            }
        }
#pragma endregion
#define SIZE_OF_MAT4 16
        std::vector<std::string> raw_inv_bind_matrices = Common::split_string(controller_source_list[1]->get_tokens("float_array")[0]->data, ' ');
        std::vector<glm::mat4> inv_bind_matrices;
        inv_bind_matrices.reserve(joint_names.size());
        for (unsigned int i = 0; i < joint_names.size(); ++i) {
            const unsigned int base_index = i * SIZE_OF_MAT4;
            inv_bind_matrices.emplace_back(
                std::stof(raw_inv_bind_matrices[base_index + 0]), std::stof(raw_inv_bind_matrices[base_index + 4]), std::stof(raw_inv_bind_matrices[base_index + 8]), std::stof(raw_inv_bind_matrices[base_index + 12]),
                std::stof(raw_inv_bind_matrices[base_index + 1]), std::stof(raw_inv_bind_matrices[base_index + 5]), std::stof(raw_inv_bind_matrices[base_index + 9]), std::stof(raw_inv_bind_matrices[base_index + 13]),
                std::stof(raw_inv_bind_matrices[base_index + 2]), std::stof(raw_inv_bind_matrices[base_index + 6]), std::stof(raw_inv_bind_matrices[base_index + 10]), std::stof(raw_inv_bind_matrices[base_index + 14]),
                std::stof(raw_inv_bind_matrices[base_index + 3]), std::stof(raw_inv_bind_matrices[base_index + 7]), std::stof(raw_inv_bind_matrices[base_index + 11]), std::stof(raw_inv_bind_matrices[base_index + 15])
            );
        }

        joint_name_to_id;
        joint_name_to_id.reserve(joint_names.size());
        for (unsigned int i = 0; i < joint_names.size(); ++i) {
            joint_name_to_id[joint_names[i]] = i;
        }

        SimpleJoint* all_joints = new SimpleJoint[joint_names.size()];
        local_bind_transforms = new glm::mat4[joint_names.size()];

        for (unsigned int i = 0; i < joint_names.size(); i++) {
            local_bind_transforms[i] = glm::mat4(1.0f);
        }

        Token* library_visual_scenes = collada->get_tokens("library_visual_scenes")[0];
        Token* root_node = library_visual_scenes->get_tokens("node", 2)[0];
        SimpleJoint root;
        root.name = "root";
        root.id = -1;
        std::pair<SimpleJoint*, Token*> joint_node;
        std::stack<std::pair<SimpleJoint*, Token*>> todo;
        todo.push({&root, root_node});

        std::vector<int> done;

        while (!todo.empty()) {
            auto current = todo.top();
            todo.pop();
            auto joint = current.first;
            auto node = current.second;

            auto matrix = node->get_tokens("matrix", 1);
            if (!matrix.empty()) {
                std::vector<std::string> raw_bind_mat = Common::split_string(matrix[0]->data, ' ');
                local_bind_transforms[joint->id] = glm::mat4(
                    std::stof(raw_bind_mat[0]), std::stof(raw_bind_mat[4]), std::stof(raw_bind_mat[8]), std::stof(raw_bind_mat[12]),
                    std::stof(raw_bind_mat[1]), std::stof(raw_bind_mat[5]), std::stof(raw_bind_mat[9]), std::stof(raw_bind_mat[13]),
                    std::stof(raw_bind_mat[2]), std::stof(raw_bind_mat[6]), std::stof(raw_bind_mat[10]), std::stof(raw_bind_mat[14]),
                    std::stof(raw_bind_mat[3]), std::stof(raw_bind_mat[7]), std::stof(raw_bind_mat[11]), std::stof(raw_bind_mat[15])
                );
            }

            auto child_nodes = node->get_tokens("node", 1);
            for (const auto& child : child_nodes) {
                if (joint_name_to_id.find(child->props["id"]) == joint_name_to_id.end()) {
                    continue;
                }
                SimpleJoint* target = &all_joints[joint_name_to_id[child->props["id"]]];
                target->id = joint_name_to_id[child->props["id"]];
                target->name = child->props["id"];
                joint->children.push_back(target);
                todo.push({ target, child });
                done.push_back(target->id);
            }
        }

        Token* library_animations = collada->get_tokens("library_animations")[0];
        animation_list = library_animations->get_tokens("animation");
        current_pose = new glm::mat4[joint_names.size()];
        // reset the pose... Incase some joints have no animation.....
        for (unsigned int i = 0; i < joint_names.size(); i++) {
            current_pose[i] = local_bind_transforms[i];
        }
        
        for (const auto& animation : animation_list) {
            Token* channel = animation->get_tokens("channel")[0];
            std::string joint_name = Common::split_string(channel->props["target"], '/')[0];

            if (joint_name_to_id.find(joint_name) == joint_name_to_id.end()) {
                continue;
            }

            unsigned int joint_id = joint_name_to_id[joint_name];
            const int base_index = the_pose_I_wanna_show * SIZE_OF_MAT4;

            std::vector<Token*> source_list = animation->get_tokens("source");
            Token* key_frame_float_array = source_list[1]->get_tokens("float_array")[0];
            std::string raw_keyframes = key_frame_float_array->data;
            std::vector<float> keyframes;
            keyframes.reserve(std::stoi(key_frame_float_array->props["count"]));
            for (const auto& s : Common::split_string(raw_keyframes, ' ')) {
                keyframes.push_back(std::stof(s));
            }

            current_pose[joint_id] = glm::mat4(
                keyframes[base_index + 0], keyframes[base_index + 4], keyframes[base_index + 8], keyframes[base_index + 12],
                keyframes[base_index + 1], keyframes[base_index + 5], keyframes[base_index + 9], keyframes[base_index + 13],
                keyframes[base_index + 2], keyframes[base_index + 6], keyframes[base_index + 10], keyframes[base_index + 14],
                keyframes[base_index + 3], keyframes[base_index + 7], keyframes[base_index + 11], keyframes[base_index + 15]
            );

        }


        unsigned int* c_indices = new unsigned int[indices.size()];
        std::copy(indices.begin(), indices.end(), c_indices);

        float * c_vertices = new float[vertices.size()];
        std::copy(vertices.begin(), vertices.end(), c_vertices);

#pragma region Render stuff
        Renderer& renderer = Renderer::get_instance();
        renderer.init(
            "3d animated model Demo",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            true,
            false
        );
        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        cam.set_position({ 0, -10, 5 });
        cam.set_rotation({ PI / 2, 0, 0 });

        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        layout.push<float>(2); // uv
        layout.push<int>(2); // joints
        layout.push<int>(2); // joints
        layout.push<int>(2); // joints
        layout.push<int>(2); // joints
        layout.push<int>(2); // joints
        layout.push<float>(2); // weights
        layout.push<float>(2); // weights
        layout.push<float>(2); // weights
        layout.push<float>(2); // weights
        layout.push<float>(2); // weights
        // Shader shader("Test");
        // Shader shader("BlinnPhong");
        // Shader shader("TexturedBlinnPhong");
        Shader shader("AnimatedBlinnPhong");
        Mesh mesh(c_vertices, c_indices, vertices.size() * sizeof(float), indices.size(), layout);
        // Texture2D texture("Dragon_ground_color.jpg");
        Texture2D texture(texture_file_name);
        Model model(&mesh, &texture, &shader, nullptr, 0);

        
        glm::mat4* animated_matrices = new glm::mat4[joint_names.size()];
        for (unsigned int i = 0; i < joint_names.size(); i++) {
            animated_matrices[i] = glm::mat4(1.0f);
        }

        // debugging
        // _print_matrices(std::vector<glm::mat4>(animated_matrices, animated_matrices + sizeof(animated_matrices)/sizeof(animated_matrices[0])));
        // Print first 5 vertices only
        // _print_vertex_data(vertices.data(), size_t(5));


        Timer timer;
        float time_of_last_frame = 0;
        while (!renderer.is_terminated()) {
            float delta_time = float(timer.GetTime()) - time_of_last_frame;
            while (delta_time < 125000.0f) {
                delta_time = float(timer.GetTime()) - time_of_last_frame;
            }

            time_of_last_frame = float(timer.GetTime());
            // model.set_scale({20, 20, 20});
            _handle_free_camera_inputs(renderer, cam);

            root.update_joint_transform(&current_pose[0], animated_matrices, -1);
            
            renderer.begin_draw();
            glm::vec3 light_pos = cam.get_position();

            shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());

            shader.set_uniform_3f("u_view_pos", cam.get_position());
            shader.set_uniform_3f("u_light_pos", light_pos);
            shader.set_uniform_3f("u_light_color", { 1, 1, 1 });
            shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            // new stuff for animation
            // shader.set_uniform_mat4f_array("u_joint_transforms", animated_matrices[0], 16);
            shader.set_uniform_mat4f_array("u_joint_transforms", animated_matrices[0], joint_names.size());
            // shader.set_uniform_mat4f_array("u_inv_bind", inv_bind_matrices[0], 16);
            shader.set_uniform_mat4f_array("u_inv_bind", inv_bind_matrices[0], joint_names.size());

            // renderer.draw(mesh.m_vao, mesh.m_ibo, shader);
            renderer.draw(model);
            renderer.end_draw();
        }
#pragma endregion
        delete[] c_vertices;
        delete[] c_indices;
        delete[] current_pose;
        delete[] all_joints;
        delete[] animated_matrices;
        delete[] local_bind_transforms;
    };


//     void do_stuff() {
//         std::string xml_content = FileSystem::read_file("objs/thin_mat_model.dae");
        
//         std::vector<Token> tokens;
//         try {
//             tokens = tokenize(xml_content);
//         } catch (const std::exception& e) {
//             const std::string message = std::string(e.what());
//             Log::log_error_and_terminate(message, __FILE__, __LINE__);
//         }

//         Token* collada = &tokens[0];
//         Token* library_geometries = collada->get_tokens("library_geometries")[0];
//         std::vector<Token*> geometry_list = library_geometries->get_tokens("geometry");

//         Token* library_controllers = collada->get_tokens("library_controllers")[0];

// #pragma region trying to rig skeleton
//         std::string raw_joint_names = library_controllers->get_tokens("Name_array", {{"id", "Armature_Cube-skin-joints-array"}})[0]->data;
//         std::vector<std::string> joint_names = Common::split_string(raw_joint_names, ' ');

//         std::string raw_weights = library_controllers->get_tokens("float_array", {{"id", "Armature_Cube-skin-weights-array"}})[0]->data;
//         std::vector<float> weights;
//         for (const auto& s : Common::split_string(raw_weights, ' ')) {
//             weights.push_back(std::stof(s));
//         }

//         Token* vertex_weights = library_controllers->get_tokens("vertex_weights")[0];
//         std::string raw_joint_weight_vcounts = vertex_weights->get_tokens("vcount")[0]->data;
//         std::vector<unsigned int> joint_weight_vcounts;
//         for (const auto& s : Common::split_string(raw_joint_weight_vcounts, ' ')) {
//             joint_weight_vcounts.push_back(std::stoi(s));
//         }

//         std::string raw_joint_weight_indices = vertex_weights->get_tokens("v")[0]->data;
//         std::vector<unsigned int> bad_joint_weight_indices;
//         for (const auto& s : Common::split_string(raw_joint_weight_indices, ' ')) {
//             bad_joint_weight_indices.push_back(std::stoi(s));
//         }

//         std::vector<int> joint_weight_indices;
//         joint_weight_indices.reserve((10 + 10) * joint_weight_vcounts.size()); // at most 10 joint and 10 weight per vertex.
//         unsigned int base_of_joint_weight = 0;
//         for (unsigned int vertex_id = 0; vertex_id < joint_weight_vcounts.size(); ++vertex_id) {
//             const auto joint_count = joint_weight_vcounts[vertex_id];

//             // We need to add padding here so that I can use a regular stride to index joints and weights, 
//             // and push them to the VBO.
//             for (unsigned int offset = 0; offset < 10; ++offset) { 
//                 // add joint indices
//                 if (offset < joint_count) {
//                     joint_weight_indices.push_back(bad_joint_weight_indices[base_of_joint_weight + 2 * offset]);
//                 } else {
//                     // add some padding
//                     joint_weight_indices.push_back(-1);
//                 }
//             }

//             for (unsigned int offset = 0; offset < 10; ++offset) { 
//                 if (offset < joint_count) {
//                     // add weight indices
//                     joint_weight_indices.push_back(bad_joint_weight_indices[base_of_joint_weight + 2 * offset + 1]);
//                 } else {
//                     // add some padding
//                     joint_weight_indices.push_back(-1);
//                 }
//             }

//             base_of_joint_weight += 2 * joint_count;
//         }

// #pragma endregion

// #pragma region building the vertex buffer
//         std::vector<unsigned int> indices;
//         std::vector<float> vertices;
//         unsigned int i_curret_vertex = 0;

//         for (const auto& geometry : geometry_list) {
//             std::vector<Token*> polylist_list = geometry->get_tokens("polylist");
//             for (const auto& polylist : polylist_list) {
                
//                 std::string raw_indices = polylist->get_tokens("p")[0]->data;
//                 std::vector<unsigned int> collada_indices;
//                 for (const auto& s : Common::split_string(raw_indices, ' ')) {
//                     collada_indices.push_back(std::stoi(s));
//                 }
                
//                 std::string raw_vcount = polylist->get_tokens("vcount")[0]->data;
//                 std::vector<unsigned int> vcounts;
//                 for (const auto& s : Common::split_string(raw_vcount, ' ')) {
//                     vcounts.push_back(std::stoi(s));
//                 }

//                 std::vector<Token*> source_list = geometry->get_tokens("source");
//                 std::string raw_positions = source_list[0]->get_tokens("float_array")[0]->data;
//                 std::vector<float> positions;
//                 for (const auto& s : Common::split_string(raw_positions, ' ')) {
//                     positions.push_back(std::stof(s));
//                 }

//                 std::string raw_normals = source_list[1]->get_tokens("float_array")[0]->data;
//                 std::vector<float> normals;
//                 for (const auto& s : Common::split_string(raw_normals, ' ')) {
//                     normals.push_back(std::stof(s));
//                 }

//                 std::string raw_texcoords0 = source_list[2]->get_tokens("float_array")[0]->data;
//                 std::vector<float> texcoords0;
//                 for (const auto& s : Common::split_string(raw_texcoords0, ' ')) {
//                     texcoords0.push_back(std::stof(s));
//                 }

//                 const auto elements_in_an_index_bundle = 4; // pos norm tex colour (thin matrix model)
//                 for (unsigned int i = 0; i < vcounts.size(); ++i) {
//                     auto count = vcounts[i];
//                     const auto base_index_bundle = i * elements_in_an_index_bundle * count; // there are 4 vertex. Each has pos and norm.
                    
//                     for (auto& offset : { 0, 1, 2 }) {
//                         const auto i_pos = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
//                         const auto i_norm = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 1];
//                         const auto i_tex = 2 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 2];
//                         vertices.push_back(positions[i_pos]);     // x
//                         vertices.push_back(positions[i_pos + 1]); // y
//                         vertices.push_back(positions[i_pos + 2]); // z
//                         vertices.push_back(normals[i_norm]);     // x
//                         vertices.push_back(normals[i_norm + 1]); // y
//                         vertices.push_back(normals[i_norm + 2]); // z
//                         vertices.push_back(texcoords0[i_tex]);
//                         vertices.push_back(texcoords0[i_tex + 1]);

//                         // i_pos is basically the vertex id. Now we should be able to get exactly the joint and weight offset.
//                         const auto base_index_joint_weight = 20 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
//                         for (unsigned int j = 0; j < 20; ++j) {
//                             if (j < 10) { // joint
//                                 int joint_id = joint_weight_indices[base_index_joint_weight + j];
//                                 vertices.push_back(reinterpret_cast<float&>(joint_id));
//                             } else { // weight
//                                 const int weight_id = joint_weight_indices[base_index_joint_weight + j];
//                                 if (weight_id < 0) {
//                                     vertices.push_back(0.0f);
//                                 } else {
//                                     const float weight = weights[weight_id];
//                                     vertices.push_back(weight);
//                                 }
//                             }
//                         }
                        
//                         indices.push_back(i_curret_vertex++);
//                     }
//                     if (count > 3) { // it is a quad
//                         for (auto& offset : { 0, 2, 3 }) {
//                             const auto i_pos = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
//                             const auto i_norm = 3 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 1];
//                             const auto i_tex = 2 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle + 2];
//                             vertices.push_back(positions[i_pos]);     // x
//                             vertices.push_back(positions[i_pos + 1]); // y
//                             vertices.push_back(positions[i_pos + 2]); // z
//                             vertices.push_back(normals[i_norm]);     // x
//                             vertices.push_back(normals[i_norm + 1]); // y
//                             vertices.push_back(normals[i_norm + 2]); // z
//                             vertices.push_back(texcoords0[i_tex]);
//                             vertices.push_back(texcoords0[i_tex + 1]);

//                             // i_pos is basically the vertex id. Now we should be able to get exactly the joint and weight offset.
//                             const auto base_index_joint_weight = 20 * collada_indices[base_index_bundle + offset * elements_in_an_index_bundle];
//                             for (unsigned int j = 0; j < 20; ++j) {
//                                 if (j < 10) { // joint
//                                     const float joint_id = joint_weight_indices[base_index_joint_weight + j];
//                                     vertices.push_back(joint_id);
//                                 } else { // weight
//                                     const int weight_id = joint_weight_indices[base_index_joint_weight + j];
//                                     if (weight_id < 0) {
//                                         vertices.push_back(0.0f);
//                                     } else {
//                                         const float weight = weights[weight_id];
//                                         vertices.push_back(weight);
//                                     }
//                                 }
//                             }

//                             indices.push_back(i_curret_vertex++);
//                         }
//                     }
//                 }
//             }
//         }
// #pragma endregion

//         std::vector<std::string> raw_inv_bind_matrices = Common::split_string(library_controllers->get_tokens("float_array", {{ "id", "Armature_Cube-skin-bind_poses-array" }})[0]->data, ' ');
//         std::vector<glm::mat4> inv_bind_matrices;
//         inv_bind_matrices.reserve(joint_names.size());
//         for (unsigned int i = 0; i < joint_names.size(); ++i) {
//             const unsigned int base_index = i * 16;
//             inv_bind_matrices.emplace_back(
//                 std::stof(raw_inv_bind_matrices[base_index + 0]), std::stof(raw_inv_bind_matrices[base_index + 4]), std::stof(raw_inv_bind_matrices[base_index + 8]), std::stof(raw_inv_bind_matrices[base_index + 12]),
//                 std::stof(raw_inv_bind_matrices[base_index + 1]), std::stof(raw_inv_bind_matrices[base_index + 5]), std::stof(raw_inv_bind_matrices[base_index + 9]), std::stof(raw_inv_bind_matrices[base_index + 13]),
//                 std::stof(raw_inv_bind_matrices[base_index + 2]), std::stof(raw_inv_bind_matrices[base_index + 6]), std::stof(raw_inv_bind_matrices[base_index + 10]), std::stof(raw_inv_bind_matrices[base_index + 14]),
//                 std::stof(raw_inv_bind_matrices[base_index + 3]), std::stof(raw_inv_bind_matrices[base_index + 7]), std::stof(raw_inv_bind_matrices[base_index + 11]), std::stof(raw_inv_bind_matrices[base_index + 15])
//             );
//             // inv_bind_matrices.emplace_back(1.0f);
//         }

//         // _print_matrices(inv_bind_matrices);

//         joint_name_to_id;
//         joint_name_to_id.reserve(joint_names.size());
//         for (unsigned int i = 0; i < joint_names.size(); ++i) {
//             joint_name_to_id[joint_names[i]] = i;
//         }

        

//         SimpleJoint all_joints[16];
//         // Token* library_animations = collada->get_tokens("library_animations")[0];
//         Token* library_visual_scenes = collada->get_tokens("library_visual_scenes")[0];
//         Token* root_node = collada->get_tokens("node", {{ "id", "Armature" }})[0];
//         SimpleJoint root;
//         root.name = "root";
//         root.id = -1;
//         std::pair<SimpleJoint*, Token*> joint_node;
//         std::stack<std::pair<SimpleJoint*, Token*>> todo;
//         todo.push({&root, root_node});
//         while (!todo.empty()) {
//             auto current = todo.top();
//             todo.pop();
//             auto joint = current.first;
//             auto node = current.second;

//             auto child_nodes = node->get_tokens("node", 1);
//             for (const auto& child : child_nodes) {
//                 SimpleJoint* target = &all_joints[joint_name_to_id[child->props["id"]]];
//                 target->id = joint_name_to_id[child->props["id"]];
//                 target->name = child->props["id"];
//                 joint->children.push_back(target);
//                 todo.push({ target, child });
//             }
//         }

//         Token* library_animations = collada->get_tokens("library_animations")[0];
//         animation_list = library_animations->get_tokens("animation");
//         // std::vector<glm::mat4> current_pose;
//         // current_pose.reserve(joint_names.size());
        
//         for (const auto& animation : animation_list) {
//             Token* channel = animation->get_tokens("channel")[0];
//             std::string joint_name = Common::split_string(channel->props["target"], '/')[0];
//             unsigned int joint_id = joint_name_to_id[joint_name];
//             const int base_index = the_pose_I_wanna_show * 16;

//             std::vector<Token*> source_list = animation->get_tokens("source");
//             std::string raw_keyframes = source_list[1]->get_tokens("float_array")[0]->data;
//             std::vector<float> keyframes;
//             keyframes.reserve(80);
//             for (const auto& s : Common::split_string(raw_keyframes, ' ')) {
//                 keyframes.push_back(std::stof(s));
//             }

//             current_pose[joint_id] = glm::mat4(
//                 keyframes[base_index + 0], keyframes[base_index + 4], keyframes[base_index + 8], keyframes[base_index + 12],
//                 keyframes[base_index + 1], keyframes[base_index + 5], keyframes[base_index + 9], keyframes[base_index + 13],
//                 keyframes[base_index + 2], keyframes[base_index + 6], keyframes[base_index + 10], keyframes[base_index + 14],
//                 keyframes[base_index + 3], keyframes[base_index + 7], keyframes[base_index + 11], keyframes[base_index + 15]
//             );

//             // current_pose[joint_id] = glm::mat4(1.0f);
//         }


//         unsigned int* c_indices = new unsigned int[indices.size()];
//         std::copy(indices.begin(), indices.end(), c_indices);

//         float * c_vertices = new float[vertices.size()];
//         std::copy(vertices.begin(), vertices.end(), c_vertices);

// #pragma region Render stuff
//         Renderer& renderer = Renderer::get_instance();
//         renderer.init(
//             "3d model Demo",
//             WINDOW_WIDTH,
//             WINDOW_HEIGHT,
//             true,
//             false
//         );
//         Camera cam(renderer.get_window_width(), renderer.get_window_height());
//         cam.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });

//         VertexBufferLayout layout;
//         layout.push<float>(3); // position
//         layout.push<float>(3); // normal
//         layout.push<float>(2); // uv
//         layout.push<int>(2); // joints
//         layout.push<int>(2); // joints
//         layout.push<int>(2); // joints
//         layout.push<int>(2); // joints
//         layout.push<int>(2); // joints
//         layout.push<float>(2); // weights
//         layout.push<float>(2); // weights
//         layout.push<float>(2); // weights
//         layout.push<float>(2); // weights
//         layout.push<float>(2); // weights
//         // Shader shader("Test");
//         // Shader shader("BlinnPhong");
//         // Shader shader("TexturedBlinnPhong");
//         Shader shader("AnimatedBlinnPhong");
//         Mesh mesh(c_vertices, c_indices, vertices.size() * sizeof(float), indices.size(), layout);
//         Texture2D texture("thin_mat_model.png");
//         Model model(&mesh, &texture, &shader, nullptr, 0);

        
//         glm::mat4 animated_matrices[16];
        
//         // debugging
//         _print_matrices(std::vector<glm::mat4>(animated_matrices, animated_matrices + sizeof(animated_matrices)/sizeof(animated_matrices[0])));
//         // Print first 5 vertices only
//         _print_vertex_data(vertices.data(), size_t(5));


//         Timer timer;
//         float time_of_last_frame = 0;
//         while (!renderer.is_terminated()) {
//             float delta_time = float(timer.GetTime()) - time_of_last_frame;
//             while (delta_time < 125000.0f) {
//                 delta_time = float(timer.GetTime()) - time_of_last_frame;
//             }

//             time_of_last_frame = float(timer.GetTime());
//             model.update();
//             _handle_free_camera_inputs(renderer, cam);

//             root.update_joint_transform(&current_pose[0], &animated_matrices[0], -1);
            
//             renderer.begin_draw();
//             glm::vec3 light_pos = cam.get_position();

//             shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());

//             shader.set_uniform_3f("u_view_pos", cam.get_position());
//             shader.set_uniform_3f("u_light_pos", light_pos);
//             shader.set_uniform_3f("u_light_color", { 1, 1, 1 });
//             shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

//             // new stuff for animation
//             shader.set_uniform_mat4f_array("u_joint_transforms", animated_matrices[0], 16);
//             shader.set_uniform_mat4f_array("u_inv_bind", inv_bind_matrices[0], 16);

//             // renderer.draw(mesh.m_vao, mesh.m_ibo, shader);
//             renderer.draw(model);
//             renderer.end_draw();
//         }
// #pragma endregion
//         delete[] c_vertices;
//         delete[] c_indices;
        
//     };
};