#version 330 core

uniform mat4 u_model;
uniform float u_health_percentage;

layout(location = 0) in vec3 in_position;
layout(location = 3) in vec2 in_uv;

out vec2 v_uv;

void main() {
    v_uv = in_uv;
    v_uv.y *= u_health_percentage;
	gl_Position = u_model * vec4(in_position, 1.0);
}