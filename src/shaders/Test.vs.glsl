#version 330 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_norm;

uniform mat4 u_view_project;
uniform mat4 u_model;

out vec3 v_normal;
out vec3 v_frag_pos;

void main() {
    gl_Position = u_view_project * u_model * vec4(in_pos, 1.0);
}