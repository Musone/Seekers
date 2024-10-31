#version 330 core

in vec3 v_normal;
in vec3 v_frag_pos;
in vec2 v_uv;

uniform vec3 u_light_pos;
uniform vec3 u_view_pos;  // Camera position
uniform vec3 u_light_color;
uniform vec3 u_object_color;
uniform sampler2D u_texture;

out vec4 frag_color;

void main() {
    // Check texture transparency
    vec4 tex_color = texture(u_texture, v_uv);
    if (tex_color.w < 0.01) {
        discard;
    }

    // Normalize vectors
    vec3 norm = normalize(v_normal);
    vec3 light_dir = normalize(u_light_pos - v_frag_pos);
    vec3 view_dir = normalize(u_view_pos - v_frag_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);

    // Ambient
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * u_light_color;

    // Diffuse
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * u_light_color;

    // Specular
    float specular_strength = 0.5;
    float spec = pow(max(dot(norm, halfway_dir), 0.0), 32.0);
    vec3 specular = specular_strength * spec * u_light_color;

    // vec3 result = (ambient + diffuse + specular) * u_object_color;

    vec3 result = (ambient + diffuse + specular) * tex_color.xyz;
    result = clamp(result, 0.0, 1.0);
    frag_color = vec4(result, tex_color.w);
    // frag_color = vec4(1.0, 0.0, 0.0, 1.0);
    // frag_color = texture(u_texture, v_uv);
}
