#version 330 core

uniform samplerCube u_skybox;

in vec3 v_uv;

out vec4 FragColor;

void main() {    
    FragColor = texture(u_skybox, v_uv);
}