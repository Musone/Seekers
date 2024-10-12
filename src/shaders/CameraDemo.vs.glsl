#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 colour;

out vec3 v_colour;

uniform mat4 u_view_project;

void main() {
	v_colour = colour;
	gl_Position = u_view_project * vec4(position.xyz, 1.0);
}