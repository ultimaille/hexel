#version 330 core

// For each pixel, a few "slices" (half-planes containing the view vector) are marched in both screen directions
// to find the highest horizon on each side. The cosine-weighted visible arc between the two horizons is integrated
// analytically around the projected normal. Assumes an orthographic projection: the view vector is (0,0,1)
// everywhere and the search radius has the same pixel size everywhere on screen.

uniform sampler2D source_depth;
// orthographic projection: view-space coordinates are affine in uv and depth
uniform vec2  depth_to_z;   // z_view  = depth_to_z.x * ndc_z + depth_to_z.y
uniform vec4  uv_to_view;   // xy_view = uv * uv_to_view.xy + uv_to_view.zw
uniform float radius_px;    // search radius, in pixels
uniform float falloff_mul;  // occluder weight = clamp(distance * falloff_mul + falloff_add, 0, 1)
uniform float falloff_add;
uniform float power;        // contrast: visibility^power
uniform int   slice_count;
uniform int   step_count;

out vec4 FragColor;

const float PI = 3.14159265359;
const float HALF_PI = 1.57079632679;
const float BACKGROUND_Z = -1e6; // where nothing was drawn: far enough to never occlude

ivec2 last_pixel = textureSize(source_depth, 0) - 1;
vec2  texel = 1.0 / vec2(textureSize(source_depth, 0));

// view-space position of the center of pixel p
vec3 view_position(ivec2 p) {
    p = clamp(p, ivec2(0), last_pixel);
    float depth = texelFetch(source_depth, p, 0).r;
    float z = depth >= 1. ? BACKGROUND_Z : depth_to_z.x * (2. * depth - 1.) + depth_to_z.y;
    vec2 uv = (vec2(p) + 0.5) * texel;
    return vec3(uv * uv_to_view.xy + uv_to_view.zw, z);
}

// surface tangent at pixel p (position P) along the screen axis e: the one-sided difference with the smaller
// depth jump, so that silhouettes do not bend the normal; on the screen border, the only one available
vec3 tangent(ivec2 p, vec3 P, ivec2 e) {
    bool has_prev = all(greaterThanEqual(p - e, ivec2(0)));
    bool has_next = all(lessThanEqual(p + e, last_pixel));
    vec3 prev = P - view_position(p - e);
    vec3 next = view_position(p + e) - P;
    return has_next && (!has_prev || abs(next.z) < abs(prev.z)) ? next : prev;
}

// Jimenez 2014, "Next Generation Post Processing in Call of Duty: Advanced Warfare"
float interleaved_gradient_noise(vec2 p) {
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

void main() {
    ivec2 p = ivec2(gl_FragCoord.xy);
    vec3 P = view_position(p);
    if (P.z <= BACKGROUND_Z) {
        FragColor = vec4(1.0);
        return;
    }

    // normal from depth: on each axis, take the neighbor with the smaller depth jump so silhouettes do not bend it
    vec3 N = normalize(cross(tangent(p, P, ivec2(1, 0)), tangent(p, P, ivec2(0, 1))));
    if (N.z < 0.0) N = -N; // face the camera; V = (0,0,1)

    float slice_noise = interleaved_gradient_noise(vec2(p));
    float step_noise  = interleaved_gradient_noise(vec2(p) + vec2(5.588238, 1.234567));

    float sum = 0.0;
    for (int s = 0; s < slice_count; ++s) {
        float phi = (float(s) + slice_noise) * PI / float(slice_count);
        vec2 dir = vec2(cos(phi), sin(phi));

        // slice frame: V = (0,0,1) and T = (dir,0); project N onto the slice plane
        vec3 axis = vec3(-dir.y, dir.x, 0.0);
        vec3 projN = N - axis * dot(N, axis);
        float projN_len = length(projN);
        float cos_n = clamp(projN.z / projN_len, 0.0, 1.0);
        float n = sign(dot(projN.xy, dir)) * acos(cos_n); // signed angle from V to projN, towards T

        // horizons start at the tangent plane: angles n + pi/2 (side +T) and n - pi/2 (side -T)
        float low_cos0 = -sin(n);
        float low_cos1 =  sin(n);
        float horizon_cos0 = low_cos0;
        float horizon_cos1 = low_cos1;

        for (int j = 0; j < step_count; ++j) {
            float t = (float(j) + step_noise) / float(step_count);
            float offset_px = t * t * radius_px + 1.0; // denser near the center, never the center pixel itself
            ivec2 offset = ivec2(round(dir * offset_px));

            vec3 d0 = view_position(p + offset) - P;
            vec3 d1 = view_position(p - offset) - P;
            float len0 = length(d0);
            float len1 = length(d1);

            // occluders fade out with distance (no dark halos behind silhouettes); background is infinitely far
            float w0 = clamp(len0 * falloff_mul + falloff_add, 0.0, 1.0);
            float w1 = clamp(len1 * falloff_mul + falloff_add, 0.0, 1.0);
            horizon_cos0 = max(horizon_cos0, mix(low_cos0, d0.z / len0, w0));
            horizon_cos1 = max(horizon_cos1, mix(low_cos1, d1.z / len1, w1));
        }

        // horizon angles from V, clamped to the hemisphere around the projected normal
        float h0 = n + max(-acos(horizon_cos1) - n, -HALF_PI);
        float h1 = n + min( acos(horizon_cos0) - n,  HALF_PI);

        // cosine-weighted visible arc between h0 and h1
        float sin_n = sin(n);
        float arc0 = (cos_n + 2.0 * h0 * sin_n - cos(2.0 * h0 - n)) * 0.25;
        float arc1 = (cos_n + 2.0 * h1 * sin_n - cos(2.0 * h1 - n)) * 0.25;
        sum += projN_len * (arc0 + arc1);
    }

    FragColor = vec4(pow(clamp(sum / float(slice_count), 0.0, 1.0), power));
}
