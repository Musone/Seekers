#version 330 core

uniform sampler2D u_texture;
uniform vec3 u_scale;

in vec2 v_uv;

out vec4 FragColor;

void main() {
    FragColor = texture(u_texture, v_uv * u_scale.xy);
}