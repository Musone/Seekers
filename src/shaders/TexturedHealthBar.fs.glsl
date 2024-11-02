#version 330 core

uniform sampler2D u_texture;
uniform vec3 u_colour;

in vec2 v_uv;

out vec4 FragColour;

void main() {
    vec4 texcolour = texture(u_texture, v_uv);
    if (texcolour.w < 0.001) {
        discard;
    } 
    FragColour = vec4(texcolour.xyz * u_colour, texcolour.w);
}