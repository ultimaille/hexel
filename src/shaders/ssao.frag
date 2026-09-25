#version 330 core

in vec2 TexCoord;

uniform sampler2D source_color;
uniform sampler2D source_depth;
uniform mat4 inverse_projection;
uniform vec2 texel_size;

uniform float horizon_radius_pixels;
uniform float horizon_bias;
uniform int horizon_steps;
uniform float ao_strength;

out vec4 FragColor;

vec3 reconstruct_view_position(vec2 uv, float depth) {
    vec4 clip_position = vec4(
        uv * 2.0 - 1.0,
        depth * 2.0 - 1.0,
        1.0
    );

    vec4 view_position =
        inverse_projection * clip_position;

    return view_position.xyz / view_position.w;
}

bool outside(vec2 uv) {
    return uv.x < 0.0 ||
           uv.x > 1.0 ||
           uv.y < 0.0 ||
           uv.y > 1.0;
}

void main() {
    vec4 source = texture(source_color, TexCoord);
    float center_depth = texture(source_depth, TexCoord).r;

    // Keep pixels with no geometry unchanged.
    if (center_depth >= 0.999999) {
        FragColor = source;
        return;
    }

    vec3 center =
        reconstruct_view_position(
            TexCoord,
            center_depth
        );

    /*
     * Estimate the view-space normal from two neighboring positions.
     */
    vec2 uv_x = TexCoord + vec2(texel_size.x, 0.0);
    vec2 uv_y = TexCoord + vec2(0.0, texel_size.y);

    if (outside(uv_x) || outside(uv_y)) {
        FragColor = source;
        return;
    }

    float depth_x = texture(source_depth, uv_x).r;
    float depth_y = texture(source_depth, uv_y).r;

    if (depth_x >= 0.999999 || depth_y >= 0.999999) {
        FragColor = source;
        return;
    }

    vec3 position_x =
        reconstruct_view_position(uv_x, depth_x);

    vec3 position_y =
        reconstruct_view_position(uv_y, depth_y);

    vec3 normal = normalize(
        cross(position_x - center, position_y - center)
    );

    /*
     * Orient the normal toward the camera. The camera is at the origin
     * in view space, so -center points approximately toward it.
     */
    if (dot(normal, -center) < 0.0) {
        normal = -normal;
    }

    /*
     * One-direction horizon search:
     *
     *     center → +screen-X
     *
     * horizon stores the greatest elevation of a sampled surface relative
     * to the current surface normal.
     */
    float horizon = 0.0;
    float valid_samples = 0.0;

    vec2 screen_direction = vec2(1.0, 0.0);

    for (int i = 1; i <= 32; ++i) {
        if (i > horizon_steps) {
            break;
        }

        float t =
            float(i) / float(horizon_steps);

        vec2 sample_uv =
            TexCoord +
            screen_direction *
            texel_size *
            (t * horizon_radius_pixels);

        if (outside(sample_uv)) {
            break;
        }

        float sample_depth =
            texture(source_depth, sample_uv).r;

        if (sample_depth >= 0.999999) {
            continue;
        }

        vec3 sample_position =
            reconstruct_view_position(
                sample_uv,
                sample_depth
            );

        vec3 offset =
            sample_position - center;

        float distance_to_sample =
            length(offset);

        if (distance_to_sample < 0.000001) {
            continue;
        }

        vec3 direction =
            offset / distance_to_sample;

        /*
         * Positive values mean that the sampled surface rises into the
         * current surface's normal hemisphere.
         */
        float elevation =
            dot(normal, direction);

        horizon = max(horizon, elevation);
        valid_samples += 1.0;
    }

    if (valid_samples == 0.0) {
        FragColor = source;
        return;
    }

    /*
     * Remove a small amount of self-occlusion caused by depth precision
     * and normal reconstruction.
     */
    float occlusion = max(
        horizon - horizon_bias,
        0.0
    );

    float ao = clamp(
        1.0 - ao_strength * occlusion,
        0.0,
        1.0
    );

    /*
     * Debug output:
     *
     * FragColor = vec4(vec3(1.0 - occlusion), 1.0);
     */

    FragColor = vec4(
        source.rgb * ao,
        source.a
    );
    FragColor = vec4(vec3(1.0 - occlusion), 1.0);
}
