#version 330 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_uv;

out vec3 v_uv;

uniform mat4 u_view_project;

void main() {
    v_uv = in_pos;
    vec4 pos = u_view_project * vec4(in_pos, 1.0);
    gl_Position = pos.xyww;
}