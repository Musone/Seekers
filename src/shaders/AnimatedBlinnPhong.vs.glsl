#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in ivec4 in_joint_indices;  // 4 joint indices
layout(location = 4) in vec4 in_weights;         // 4 weights

uniform mat4 u_view_project;
uniform mat4 u_model;
uniform mat4 u_joint_transforms[120];

out vec3 v_normal;
out vec3 v_frag_pos;
out vec2 v_uv;

void main() {
    v_uv = in_uv;
    vec4 total_local_pos = vec4(0.0);
    vec4 total_normal = vec4(0.0);

    // Process all 4 joints
    for (int i = 0; i < 4; i++) {
        int joint_index = in_joint_indices[i];
        float weight = in_weights[i];

        if (joint_index > -1 && weight > 0.0) {
            mat4 transform = u_joint_transforms[joint_index];
            total_local_pos += transform * vec4(in_pos, 1.0) * weight;
            total_normal += transform * vec4(in_norm, 0.0) * weight;
        }
    }

    v_frag_pos = vec3(u_model * total_local_pos);
    v_normal = mat3(transpose(inverse(u_model))) * total_normal.xyz;
    gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
}

// void main() {
//     v_uv = in_uv;
//     v_frag_pos = vec3(u_model * vec4(in_pos, 1.0));
//     v_normal = mat3(transpose(inverse(u_model))) * in_norm;
//     gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
// }
