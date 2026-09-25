#version 330 core

in vec2 TexCoord;

uniform sampler2D source_color;
uniform sampler2D source_depth;
uniform mat4 inverse_projection;
uniform vec2 texel_size;

out vec4 FragColor;

vec3 reconstruct_view_position(vec2 uv, float depth) {
    vec4 clip_position = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view_position = inverse_projection * clip_position;
    return view_position.xyz / view_position.w;
}

bool outside(vec2 uv) {
    return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

void main() {
    vec4 source = texture(source_color, TexCoord);
    float center_depth = texture(source_depth, TexCoord).r;

    // Keep the background unchanged.
    if (center_depth >= 1.0) {
        FragColor = source;
        return;
    }

    vec3 center = reconstruct_view_position(TexCoord, center_depth);

    /*
     * Estimate the surface normal from neighboring reconstructed positions.
     */
    vec3 px = reconstruct_view_position(
        TexCoord + vec2(texel_size.x, 0.0),
        texture(
            source_depth,
            TexCoord + vec2(texel_size.x, 0.0)
        ).r
    );

    vec3 py = reconstruct_view_position(
        TexCoord + vec2(0.0, texel_size.y),
        texture(
            source_depth,
            TexCoord + vec2(0.0, texel_size.y)
        ).r
    );

    vec3 normal = normalize(
        cross(px - center, py - center)
    );

    /*
     * Use a small fixed kernel for this milestone.
     * The offsets are in pixel units.
     */
    const vec2 offsets[8] = vec2[](
        vec2( 1.0,  0.0),
        vec2(-1.0,  0.0),
        vec2( 0.0,  1.0),
        vec2( 0.0, -1.0),
        vec2( 2.0,  0.0),
        vec2(-2.0,  0.0),
        vec2( 0.0,  2.0),
        vec2( 0.0, -2.0)
    );

    float occlusion = 0.0;
    float valid_samples = 0.0;

    for (int i = 0; i < 8; ++i) {
        vec2 uv =
            TexCoord + offsets[i] * texel_size;

        if (outside(uv)) {
            continue;
        }

        float sample_depth =
            texture(source_depth, uv).r;

        if (sample_depth >= 1.0) {
            continue;
        }

        vec3 sample_position =
            reconstruct_view_position(
                uv,
                sample_depth
            );

        vec3 direction =
            sample_position - center;

        float distance_to_sample =
            length(direction);

        if (distance_to_sample < 0.00001) {
            continue;
        }

        valid_samples += 1.0;

        vec3 direction_normalized =
            direction / distance_to_sample;

        /*
         * A neighboring surface lying in the center pixel's normal
         * hemisphere contributes to occlusion. The distance falloff
         * prevents distant samples from dominating.
         */
        float facing =
            max(dot(normal, direction_normalized), 0.0);

        float falloff =
            1.0 - smoothstep(
                0.0,
                0.15,
                distance_to_sample
            );

        occlusion += facing * falloff;
    }

    if (valid_samples > 0.0) {
        occlusion /= valid_samples;
    }

    /*
     * Keep the effect deliberately exaggerated for debugging.
     */
    float strength = 2.0;
    float ao = clamp(
        1.0 - strength * occlusion,
        0.0,
        1.0
    );

    FragColor = vec4(
        source.rgb * ao,
        source.a
    );
//  FragColor = vec4(vec3(1.0 - occlusion), 1.0);

}
