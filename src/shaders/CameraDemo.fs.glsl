#version 330 core

in vec3 v_colour;

out vec4 FragColor;

void main() {
    // FragColor = vec4(0.3, 0.4, 1.0, 1.0);
    FragColor = vec4(v_colour.rgb, 1.0);
}