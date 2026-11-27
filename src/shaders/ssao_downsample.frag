#version 330 core

uniform sampler2D source_z;
uniform float depth_range; // texels farther than this behind the closest one get no weight

out float view_z;

void main() {
    ivec2 q = 2 * ivec2(gl_FragCoord.xy);
    ivec2 last = textureSize(source_z, 0) - 1;
    vec4 z = vec4(texelFetch(source_z, min(q,               last), 0).r,
                  texelFetch(source_z, min(q + ivec2(1, 0), last), 0).r,
                  texelFetch(source_z, min(q + ivec2(0, 1), last), 0).r,
                  texelFetch(source_z, min(q + ivec2(1, 1), last), 0).r);
    float closest = max(max(z.x, z.y), max(z.z, z.w));
    vec4 d = (closest - z) / depth_range;
    vec4 w = clamp(1.0 - d * d, 0.0, 1.0); // the closest texel always has weight 1
    view_z = dot(w, z) / dot(w, vec4(1.0));
}
