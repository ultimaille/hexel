#version 330 core

// Depth buffer -> view-space z (level 0 of the z pyramid).

uniform sampler2D source_depth;
uniform vec2 depth_to_z; // orthographic projection: z_view = depth_to_z.x * ndc_z + depth_to_z.y

out float view_z;

const float BACKGROUND_Z = -1e6; // where nothing was drawn: far enough to never occlude

void main() {
    float depth = texelFetch(source_depth, ivec2(gl_FragCoord.xy), 0).r;
    view_z = depth >= 1. ? BACKGROUND_Z : depth_to_z.x * (2. * depth - 1.) + depth_to_z.y;
}
