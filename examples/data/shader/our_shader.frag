#version 450

in vec3 input_uv;
out vec4 _entryPointOutput_color;

uniform sampler3D volume_texture;
uniform sampler3D grid_texture;

// Uniforms for model transformation
uniform mat4 model_matrix;

// Uniforms for camera settings
uniform mat4 viewproj;
uniform int homogeneous_ndc;

// Uniforms for configuration settings
uniform  float min_density, max_density;
uniform  vec3 ray_step;
uniform  int cell_length;
uniform  vec3 grid_uv_scale;

vec3 transform_object_to_world(vec3 v) {
    return (vec4(v, 1.0) * model_matrix).xyz;
}

float depth_from_world(vec3 v) {
    vec4 clip_pos = vec4(v, 1.0) * viewproj;
    float depth = clip_pos.z;
    return homogeneous_ndc == 1 ? (0.5 * depth) + 0.5 : depth;
}

vec4 main_pixel(vec3 uv) {
    vec4 output_color = vec4(0.0);
    float output_depth = 1.0;
    vec3 ray_pos = uv;
    vec4 cell_range;

    while (all(greaterThanEqual(ray_pos, vec3(0.0))) && all(lessThanEqual(ray_pos, vec3(1.0))) && (output_color.w < 0.99)) {
        cell_range = texture(grid_texture, ray_pos * grid_uv_scale);
        if (cell_range.x > max_density || cell_range.y <= min_density) {
            ray_pos += ray_step * float(cell_length);
            continue;
        }

        for (int i = 0; i < cell_length; i++) {
            if (any(lessThan(ray_pos, vec3(0.0))) || any(greaterThan(ray_pos, vec3(1.0))) || (output_color.w >= 0.99)) {
                break;
            }

            vec3 param = ray_pos - 0.5;
            vec3 world_ray_pos = transform_object_to_world(param);

            float density = texture(volume_texture, ray_pos).x;
            density = clamp((density - min_density) / (max_density - min_density), 0.0, 1.0);
            float prev_alpha = density * (1.0 - output_color.w);
            output_color += vec4(density, density, density, prev_alpha);
            output_color.w += prev_alpha;

            if (output_color.w >= 0.125) {
                output_depth = depth_from_world(world_ray_pos);
            }
            ray_pos += ray_step;
        }
    }

    return vec4(output_color.rgb, output_depth);
}

void main() {
    vec4 input_position = vec4(gl_FragCoord, 0.0, 1.0);
    vec3 uv = input_uv;

    vec4 output = main_pixel(uv);
    _entryPointOutput_color = output.rgb;
    gl_FragDepth = output.a;  // Use alpha for depth
}
