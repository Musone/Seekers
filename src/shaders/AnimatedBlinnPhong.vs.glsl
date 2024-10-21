#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in ivec3 in_joint_indices;
layout(locatino = 3) in vec3 in_weights;

uniform mat4 u_view_project;
uniform mat4 u_model;
uniform mat4 u_joint_transforms[50];

out vec3 v_normal;
out vec3 v_frag_pos;

void main() {
    vec4 total_local_pos = vec4(0.0);
    vec4 total_normal = vec4(0.0);

    for (int i = 0; i < 3; ++i) {
        mat4 joint_transform = u_joint_transforms[in_joint_indices[i]];
        vec4 pose_position = joint_transform * vec4(in_pos, 1.0);
        total_local_pos += pose_position * in_weights[i];

        vec4 world_normal = joint_transform * vec4(in_norm, 0.0);
        total_normal += world_normal * in_weights[i];
    }

    v_frag_pos = vec3(u_model * vec4(total_local_pos.xyz, 1.0));
    v_normal = mat3(transpose(inverse(u_model))) * total_normal.xyz;
    gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
}