#version 330 core

uniform mat4 u_mvp;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

void main() {
	gl_Position = u_mvp * vec4(position, 1.0);
}