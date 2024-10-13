#version 330 core

uniform sampler2D u_texture;
uniform vec3 u_colour;

out vec4 FragColour;

void main() {
    FragColour = vec4(u_colour, 1.0);
}