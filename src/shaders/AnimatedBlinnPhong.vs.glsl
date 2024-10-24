#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in ivec2 in_joint_indices0;
layout(location = 4) in ivec2 in_joint_indices1;
layout(location = 5) in ivec2 in_joint_indices2;
layout(location = 6) in ivec2 in_joint_indices3;
layout(location = 7) in ivec2 in_joint_indices4;
layout(location = 8) in vec2 in_weights0;
layout(location = 9) in vec2 in_weights1;
layout(location = 10) in vec2 in_weights2;
layout(location = 11) in vec2 in_weights3;
layout(location = 12) in vec2 in_weights4;

uniform mat4 u_view_project;
uniform mat4 u_model;
uniform mat4 u_joint_transforms[50];
uniform mat4 u_inv_bind[50];

out vec3 v_normal;
out vec3 v_frag_pos;
out vec2 v_uv;

void main() {
    v_uv = in_uv;
    vec4 total_local_pos = vec4(0.0);
    vec4 total_normal = vec4(0.0);

    ivec2[5] joint_indices = ivec2[5](
        in_joint_indices0,
        in_joint_indices1,
        in_joint_indices2,
        in_joint_indices3,
        in_joint_indices4
    );
    
    vec2[5] weights = vec2[5](
        in_weights0,
        in_weights1,
        in_weights2,
        in_weights3,
        in_weights4
    );

    for (int i = 0; i < 5; i++) {
        // Process X component
        if (joint_indices[i].x > -1) {
            mat4 joint_transform = u_joint_transforms[joint_indices[i].x];
            mat4 inv_bind = u_inv_bind[joint_indices[i].x];
            vec4 pose_position = joint_transform * inv_bind * vec4(in_pos, 1.0);
            total_local_pos += pose_position * weights[i].x;

            vec4 world_normal = joint_transform * inv_bind * vec4(in_norm, 0.0);
            total_normal += world_normal * weights[i].x;
        }

        // Process Y component
        if (joint_indices[i].y > -1) {
            mat4 joint_transform = u_joint_transforms[joint_indices[i].y];
            mat4 inv_bind = u_inv_bind[joint_indices[i].y];
            vec4 pose_position = joint_transform * inv_bind * vec4(in_pos, 1.0);
            total_local_pos += pose_position * weights[i].y;

            vec4 world_normal = joint_transform * inv_bind * vec4(in_norm, 0.0);
            total_normal += world_normal * weights[i].y;
        }
    }

    v_frag_pos = vec3(u_model * vec4(total_local_pos.xyz, 1.0));
    v_normal = mat3(transpose(inverse(u_model))) * total_normal.xyz;
    gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
}

// void main() {
//     v_uv = in_uv;
//     v_frag_pos = vec3(u_model * vec4(in_pos, 1.0));
//     v_normal = mat3(transpose(inverse(u_model))) * in_norm;
//     gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
// }

// void main() {
//     v_uv = in_uv;
    
//     // DEBUG: Get the joint index
//     int joint_idx = in_joint_indices0.x;
    
//     if (joint_idx > -1) {
//         // Try using the actual joint index but with matrix 0
//         // This will tell us if the problem is with matrix access
//         mat4 joint_transform = u_joint_transforms[joint_idx];  // Force matrix 0 instead of joint_idx
//         vec4 transformed_pos = joint_transform * vec4(in_pos, 1.0);
        
//         v_frag_pos = vec3(u_model * transformed_pos);
//         v_normal = mat3(transpose(inverse(u_model))) * in_norm;
//         gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
//     } else {
//         // Fallback to non-skinned
//         v_frag_pos = vec3(u_model * vec4(in_pos, 1.0));
//         v_normal = mat3(transpose(inverse(u_model))) * in_norm;
//         gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
//     }
// }