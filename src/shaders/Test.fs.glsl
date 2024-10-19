#version 330 core

in vec3 v_normal;
in vec3 v_frag_pos;

uniform vec3 u_light_pos;
uniform vec3 u_view_pos; // Camera position
uniform vec3 u_light_color;
uniform vec3 u_object_color;

out vec4 frag_color;

void main() {
    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
}