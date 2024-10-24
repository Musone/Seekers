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

#include <globals/Globals.h>

#include <utils/CalladaTokenizer.hpp>

#include <iomanip>

class Application {
    
public:
    void run_game_loop() {
        Renderer& renderer = Renderer::get_instance();
        // TODO: the game obviously...

        while (!renderer.is_terminated()) {
            throw std::runtime_error("not implemented lol");
        };
    };

    void run_demo_callada_3d_model() {

        // std::string xml_content = FileSystem::read_file("objs/Cube.dae");
        std::string xml_content = FileSystem::read_file("objs/BlackDragon/dragon.dae");
        // std::string xml_content = FileSystem::read_file("objs/sword.dae");
        
        std::vector<Token> tokens;
        try {
            tokens = tokenize(xml_content);
        } catch (const std::exception& e) {
            const std::string message = std::string(e.what());
            Log::log_error_and_terminate(message, __FILE__, __LINE__);
        }

        // Token* polylist = tokens[0].get_tokens("polylist")[0]; // cube
        // Token* polylist = tokens[0].get_tokens("polylist", {{"material", "Game_dragon_001-material"}})[0]; // dragon
        Token* collada = &tokens[0];
        Token* library_geometries = collada->get_tokens("library_geometries")[0];
        std::vector<Token*> geometry_list = library_geometries->get_tokens("geometry");

        Token* library_controllers = collada->get_tokens("library_controllers")[0];

#pragma region trying to rig skeleton
        std::string raw_joint_names = library_controllers->get_tokens("Name_array", {{"id", "a_Cube-skin-joints-array"}})[0]->data;
        std::vector<std::string> joint_names = Common::split_string(raw_joint_names, ' ');

        std::string raw_weights = library_controllers->get_tokens("float_array", {{"id", "a_Cube-skin-weights-array"}})[0]->data;
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

        std::vector<unsigned int> indices;
        std::vector<float> vertices;
        unsigned int i_curret_vertex = 0;

        for (const auto& geometry : geometry_list) {
            // Token* geometry = library_geometries->get_tokens("geometry", {{"id", "Circle-mesh"}})[0];
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

                // std::string raw_positions = tokens[0].get_tokens("float_array", {{"id", "Cube-mesh-positions-array"}})[0]->data; // cube
                // std::string raw_positions = tokens[0].get_tokens("float_array", {{"id", "Cube_004-mesh-positions-array"}})[0]->data; // dragon
                std::vector<Token*> source_list = geometry->get_tokens("source");
                std::string raw_positions = source_list[0]->get_tokens("float_array")[0]->data; // sword
                std::vector<float> positions;
                for (const auto& s : Common::split_string(raw_positions, ' ')) {
                    positions.push_back(std::stof(s));
                }

                // std::string raw_normals = tokens[0].get_tokens("float_array", {{"id", "Cube-mesh-normals-array"}})[0]->data; // cube
                // std::string raw_normals = tokens[0].get_tokens("float_array", {{"id", "Cube_004-mesh-normals-array"}})[0]->data; // dragon
                std::string raw_normals = source_list[1]->get_tokens("float_array")[0]->data; // sword
                std::vector<float> normals;
                for (const auto& s : Common::split_string(raw_normals, ' ')) {
                    normals.push_back(std::stof(s));
                }

                // std::string raw_texcoords1 = source_list[2]->get_tokens("float_array")[0]->data; // sword
                // std::string raw_texcoords0 = source_list[2]->get_tokens("float_array")[0]->data; // sword
                std::string raw_texcoords0 = source_list[3]->get_tokens("float_array")[0]->data; // sword
                std::vector<float> texcoords0;
                for (const auto& s : Common::split_string(raw_texcoords0, ' ')) {
                    texcoords0.push_back(std::stof(s));
                }

                
                // const auto elements_in_a_vertex = 2; // pos norm (cube)
                // const auto elements_in_a_vertex = 4; // pos norm tex1 tex2 (dragon)
                const auto elements_in_an_index_bundle = 3; // pos norm tex (sword)
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
                                const float joint_id = joint_weight_indices[base_index_joint_weight + j];
                                vertices.push_back(joint_id);
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
                                    const float joint_id = joint_weight_indices[base_index_joint_weight + j];
                                    vertices.push_back(joint_id);
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

#pragma region try finding the unique vertices
        /*
        auto size_of_a_vertex = 3 + 3 + 2; // This is the size of a vertex. (pos, norm, uv)
        std::set<std::string> unique_hash;
        // for (unsigned int i = 0; i < vertices.size(); ++i) {
        unsigned int i = 0;
        while (i < vertices.size()) {
            std::vector<std::string> temp;
            for (unsigned int j = 0; j < size_of_a_vertex; ++j) {
                temp.push_back(std::to_string(vertices[i + j]));
            }

            if (temp[temp.size() - 1].empty()) {
                int sdaiojdiasod = 123;
                int sdaiojdi = 123;
                int sdaiojdiwewqasod = 123;
            }

            std::string hash = Common::join_string(temp, ' ');
            unique_hash.insert(hash);
            i += size_of_a_vertex;
        }
        */
#pragma endregion

        unsigned int* c_indices = new unsigned int[indices.size()];
        std::copy(indices.begin(), indices.end(), c_indices);

        float * c_vertices = new float[vertices.size()];
        std::copy(vertices.begin(), vertices.end(), c_vertices);

        // std::cout << "Vertices:" << std::endl;
        // for (size_t i = 0; i < vertices.size(); i += 6) {  // Assuming each vertex has 6 components (3 for position, 3 for normal)
        //     std::cout << "V" << i/6 << ": Pos("
        //             << std::setw(6) << vertices[i] << ", "
        //             << std::setw(6) << vertices[i+1] << ", "
        //             << std::setw(6) << vertices[i+2] << ") Norm("
        //             << std::setw(6) << vertices[i+3] << ", "
        //             << std::setw(6) << vertices[i+4] << ", "
        //             << std::setw(6) << vertices[i+5] << ")" << std::endl;
        // }

        // std::cout << "\nIndices:" << std::endl;
        // for (size_t i = 0; i < indices.size(); i += 3) {
        //     std::cout << "Triangle " << i/3 << ": "
        //             << indices[i] << ", "
        //             << indices[i+1] << ", "
        //             << indices[i+2] << std::endl;
        // }

        // Taken from run_demo_obj_3d_model()
        Renderer& renderer = Renderer::get_instance();
        renderer.init(
            "3d model Demo",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            true,
            false
        );
        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        cam.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });

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
        Shader shader("TexturedBlinnPhong");
        Mesh mesh(c_vertices, c_indices, vertices.size() * sizeof(float), indices.size(), layout);
        Texture2D texture("Dragon_ground_color.jpg");
        Model model(&mesh, &texture, &shader, nullptr, 0);

        while (!renderer.is_terminated()) {
            model.update();
            _handle_free_camera_inputs(renderer, cam);
            
            renderer.begin_draw();
            glm::vec3 light_pos = cam.get_position();

            shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());

            shader.set_uniform_3f("u_view_pos", cam.get_position());
            shader.set_uniform_3f("u_light_pos", light_pos);
            shader.set_uniform_3f("u_light_color", { 1, 1, 0 });
            shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            // renderer.draw(mesh.m_vao, mesh.m_ibo, shader);
            renderer.draw(model);
            renderer.end_draw();
        }

        delete[] c_vertices;
        delete[] c_indices;
        
    };

    void run_demo_obj_3d_model() {
        Renderer& renderer = Renderer::get_instance();
        renderer.init(
            "3d model Demo",
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            true,
            false
        );
        Camera cam(renderer.get_window_width(), renderer.get_window_height());
        cam.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });

#pragma region load obj file
        std::string raw_data = FileSystem::read_file("objs/guy.obj");
        // std::string raw_data = FileSystem::read_file("objs/Cube.obj");
        std::vector<std::string> lines = Common::split_string(raw_data, '\n');
        std::vector<glm::vec3> unordered_vertices;
        std::vector<glm::vec3> unordered_normals;
        std::vector<glm::ivec3> unordered_indices;

        for (const auto& l : lines) {
            try {
                std::vector<std::string> tokens = Common::split_string(l, ' ');
                if (tokens.size() < 1) continue;

                if (tokens[0] == "v") {
                    unordered_vertices.emplace_back(
                        std::stof(tokens[1]), 
                        std::stof(tokens[2]), 
                        std::stof(tokens[3])
                    );
                } else if (tokens[0] == "vn") {
                    unordered_normals.emplace_back(
                        std::stof(tokens[1]), 
                        std::stof(tokens[2]), 
                        std::stof(tokens[3])
                    );
                } else if (tokens[0] == "f") {
                    for (int i = 1; i <= 3; ++i) {
                        const auto temp = Common::split_string(tokens[i], '/');
                        unordered_indices.emplace_back(
                            std::stoi(temp[0]) - 1,  // vertex index
                            temp[1].empty() ? -1 : std::stoi(temp[1]) - 1,  // texture index (if exists)
                            std::stoi(temp[2]) - 1  // normal index
                        );
                    }
                    if (tokens.size() > 4) {  // If it's a quad, add another triangle
                        unordered_indices.emplace_back(unordered_indices[unordered_indices.size() - 3]);
                        unordered_indices.emplace_back(unordered_indices[unordered_indices.size() - 2]);
                        const auto temp = Common::split_string(tokens[4], '/');
                        unordered_indices.emplace_back(
                            std::stoi(temp[0]) - 1,
                            temp[1].empty() ? -1 : std::stoi(temp[1]) - 1,
                            std::stoi(temp[2]) - 1
                        );
                    }
                } 
            } catch (const std::exception& e) {
                const std::string message = std::string(e.what());
                Log::log_error_and_terminate(message, __FILE__, __LINE__);
            }
        }

        std::vector<float> vertices;
        std::vector<unsigned int> indices;

        for (size_t i = 0; i < unordered_indices.size(); ++i) {
            const glm::vec3& pos = unordered_vertices[unordered_indices[i].x];
            const glm::vec3& norm = unordered_normals[unordered_indices[i].z];

            vertices.insert(vertices.end(), {pos.x, pos.y, pos.z, norm.x, norm.y, norm.z});
            indices.push_back(i);
        }

        float* c_vertices = new float[vertices.size()];
        std::copy(vertices.begin(), vertices.end(), c_vertices);

        unsigned int* c_indices = new unsigned int[indices.size()];
        std::copy(indices.begin(), indices.end(), c_indices);
#pragma endregion

        VertexBufferLayout layout;
        layout.push<float>(3); // position
        layout.push<float>(3); // normal
        // Shader shader("Test");
        Shader shader("BlinnPhong");
        Mesh mesh(c_vertices, c_indices, vertices.size() * sizeof(float), indices.size(), layout);
        Model model(&mesh, nullptr, &shader, nullptr, 0);

        while (!renderer.is_terminated()) {
            model.update();
            _handle_free_camera_inputs(renderer, cam);
            
            renderer.begin_draw();
            glm::vec3 light_pos = cam.get_position();

            shader.set_uniform_mat4f("u_view_project", cam.get_view_project_matrix());

            shader.set_uniform_3f("u_view_pos", cam.get_position());
            shader.set_uniform_3f("u_light_pos", light_pos);
            shader.set_uniform_3f("u_light_color", { 1, 1, 0 });
            shader.set_uniform_3f("u_object_color", { 0.5, 0.2, 1 });

            // renderer.draw(mesh.m_vao, mesh.m_ibo, shader);
            renderer.draw(model);
            renderer.end_draw();
        }

        delete[] c_vertices;
        delete[] c_indices;
    }

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
        cam.set_position({ 0, 0, CAMERA_DISTANCE_FROM_WORLD });

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
                cam.set_rotation({ 2 * PI / 6, 0, player_motion.angle });
                const auto temp = cam.rotate_to_camera_direction({ 0, 0, -1 });
                cam_dir = { temp.x, temp.y };
                cam_dir = Common::normalize(cam_dir);
                cam.set_position(glm::vec3(player_motion.position - (cam_dir * 5.0f), 7));
                the_3d_angle = PI / 2;
                if (!is_cursor_locked) { renderer.lock_cursor(); is_cursor_locked = true; }
            } else {
                cam.set_position(glm::vec3(player_motion.position, CAMERA_DISTANCE_FROM_WORLD));
                cam.set_rotation({ 0, 0, player_motion.angle });
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
                    cam.get_view_project_matrix() 
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
                    cam.get_view_project_matrix()
                    * Transform::create_scaling_matrix({ MAP_WIDTH, MAP_HEIGHT, 1 })
                );
                shader.set_uniform_1i("u_texture", map_texture_info.texture_slot_id);
                shader.set_uniform_3f("u_scale", { MAP_WIDTH / 8, MAP_HEIGHT / 8, 1});
                renderer.draw(square_vao, square_ibo, shader);

                shader.set_uniform_mat4f(
                    "u_mvp", 
                    cam.get_view_project_matrix()
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
                                cam.get_view_project_matrix()
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
                    
                    // Now we use whatever we found to render the entity... This is going to get messy...
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
                        renderer.draw(square_vao, square_ibo, health_shader);

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
                        renderer.draw(square_vao, square_ibo, health_shader);
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
                        renderer.draw(square_vao, square_ibo, health_shader);

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
                    cam.get_view_project_matrix()
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
        Texture2D player_texture("player.png");
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
        Texture2D skeleton_texture("skeleton.png");
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
