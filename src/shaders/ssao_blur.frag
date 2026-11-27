#version 330 core

// One direction of a separable bilateral blur of the half resolution occlusion.
// Plane-aware: each neighbor's z is compared with the z predicted by the local slope of the center's surface,
// so the blur follows tilted surfaces but stops at creases and silhouettes.

uniform sampler2D source_ao;
uniform sampler2D view_z;   // half resolution view-space z (level 0 of the pyramid), same grid as source_ao
uniform ivec2 direction;    // (1,0) or (0,1)
uniform float tolerance;    // distance off the plane at which a neighbor gets no weight

out vec4 FragColor;

const int   RADIUS = 4;
const float SIGMA = 2.0;
const float BACKGROUND_Z = -1e5; // ssao_z.frag writes -1e6 where nothing was drawn

ivec2 last = textureSize(source_ao, 0) - 1;

float z_at(ivec2 p) {
    return texelFetch(view_z, clamp(p, ivec2(0), last), 0).r;
}

void main() {
    ivec2 p = ivec2(gl_FragCoord.xy);
    float z = z_at(p);
    if (z < BACKGROUND_Z) {
        FragColor = vec4(1.0);
        return;
    }

    // slope of the surface along the direction: the one-sided difference with the smaller depth jump
    float next = z_at(p + direction) - z;
    float prev = z - z_at(p - direction);
    float slope = abs(next) < abs(prev) ? next : prev;

    float ao_sum = 0.0;
    float weight_sum = 0.0;
    for (int i = -RADIUS; i <= RADIUS; ++i) {
        ivec2 q = clamp(p + i * direction, ivec2(0), last);
        float off_plane = abs(z_at(q) - (z + slope * float(i)));
        float w = exp(-float(i * i) / (2.0 * SIGMA * SIGMA)) * max(0.0, 1.0 - off_plane / tolerance);
        ao_sum += w * texelFetch(source_ao, q, 0).r;
        weight_sum += w;
    }
    // the center tap is on the plane: weight_sum >= 1
    FragColor = vec4(ao_sum / weight_sum);
}
