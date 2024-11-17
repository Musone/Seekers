#version 330 core

in vec3 v_normal;
in vec3 v_frag_pos;
in vec2 v_uv;

#define MAX_LIGHTS 25

uniform vec3 u_view_pos;  // Camera position

// Light arrays
uniform vec3 u_light_positions[MAX_LIGHTS];
uniform float u_light_strengths[MAX_LIGHTS];
uniform vec3 u_light_color;
uniform int u_num_lights;

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
    vec3 view_dir = normalize(u_view_pos - v_frag_pos);

    // Initialize lighting result
    vec3 result = vec3(0.0);
    float ambient_strength = 0.005;
    // Ambient
    vec3 ambient = ambient_strength * u_light_color;
    result += ambient;

    for(int i = 0; i < u_num_lights; i++) {
        vec3 light_dir = normalize(u_light_positions[i] - v_frag_pos);
        vec3 halfway_dir = normalize(light_dir + view_dir);
        
        float max_light_range = u_light_strengths[i];
        float dist_from_light = distance(v_frag_pos, u_light_positions[i]);


        float light_strength = max_light_range / dist_from_light;
        light_strength = min(light_strength, 1.0);
        
        // Diffuse
        float diff = max(dot(norm, light_dir), 0.0) * light_strength;
        vec3 diffuse = diff * u_light_color;

        // Specular
        float specular_strength = 0.5 * light_strength;
        float spec = pow(max(dot(norm, halfway_dir), 0.0), 8.0);
        vec3 specular = specular_strength * spec * u_light_color;

        result += (diffuse + specular);
    }

    // Apply base color and clamp result
    result *= texture(u_texture, v_uv).rgb;
    result = clamp(result, 0.0, 1.0);
    frag_color = vec4(result, 1.0);
}
