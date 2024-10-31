#version 330 core

in vec3 v_normal;
in vec3 v_frag_pos;
in vec4 v_color;
in vec2 v_uv;

uniform vec3 u_light_pos;
uniform vec3 u_view_pos;
uniform vec3 u_light_color;
uniform vec3 u_object_color;
uniform vec3 u_scale;
uniform sampler2D u_texture;

uniform bool u_has_vertex_colors;
uniform bool u_has_texture;
uniform bool u_use_repeating_pattern;

out vec4 frag_color;

void main() {
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

    // Determine base color
    vec3 base_color;
    if (u_has_vertex_colors) {
        base_color = v_color.rgb;
    } else if (u_has_texture) {
        if (u_use_repeating_pattern) {
            base_color = texture(u_texture, v_uv * u_scale.xy).rgb;
        } else {
            base_color = texture(u_texture, v_uv).rgb;
        }
    } else {
        base_color = u_object_color;
    }

    vec3 result = (ambient + diffuse + specular) * base_color;
    result = clamp(result, 0.0, 1.0);
    frag_color = vec4(result, 1.0);
}