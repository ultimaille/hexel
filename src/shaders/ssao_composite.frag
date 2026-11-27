#version 330 core

// Upsample the half resolution occlusion and output it; the blend state multiplies it into the target's color.
// Each pixel mixes its 4 nearest occlusion texels with bilinear weights, damped by how far each texel's depth is
// from the pixel's own: across a silhouette, the pixel takes the occlusion of its own surface.

uniform sampler2D source_ao;    // half resolution
uniform sampler2D view_z;       // half resolution view-space z (level 0 of the pyramid): the depth of each ao texel
uniform sampler2D source_depth; // full resolution depth buffer
uniform vec2  depth_to_z;       // orthographic projection: z_view = depth_to_z.x * ndc_z + depth_to_z.y
uniform float tolerance;        // depth difference that halves a texel's weight

out vec4 FragColor;

void main() {
    ivec2 p = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(source_depth, p, 0).r;
    if (depth >= 1.) { // background: nothing to darken
        FragColor = vec4(1.0);
        return;
    }
    float z = depth_to_z.x * (2. * depth - 1.) + depth_to_z.y;

    // pixel p in half resolution texel units: texel q is the depth (and occlusion) of pixel 2q
    vec2 f = vec2(p) * 0.5;
    ivec2 q0 = ivec2(floor(f));
    vec2 t = f - vec2(q0);
    ivec2 last = textureSize(source_ao, 0) - 1;

    float ao_sum = 0.0;
    float weight_sum = 0.0;
    for (int i = 0; i < 4; ++i) {
        ivec2 o = ivec2(i & 1, i >> 1);
        ivec2 q = clamp(q0 + o, ivec2(0), last);
        vec2 b = mix(1.0 - t, t, vec2(o)); // bilinear weights
        float dz = abs(texelFetch(view_z, q, 0).r - z);
        float w = (b.x * b.y + 1e-3) / (1.0 + dz / tolerance); // + 1e-3: never all zero
        ao_sum += w * texelFetch(source_ao, q, 0).r;
        weight_sum += w;
    }
    FragColor = vec4(ao_sum / weight_sum);
}
