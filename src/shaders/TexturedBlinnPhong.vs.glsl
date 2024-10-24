#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_uv;

uniform mat4 u_view_project;
uniform mat4 u_model;

out vec3 v_normal;
out vec3 v_frag_pos;
out vec2 v_uv;

void main() {
    v_uv = in_uv;
    v_frag_pos = vec3(u_model * vec4(in_pos, 1.0));
    v_normal = mat3(transpose(inverse(u_model))) * in_norm;
    gl_Position = u_view_project * vec4(v_frag_pos, 1.0);
}