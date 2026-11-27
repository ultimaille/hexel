#version 330 core

// Depth buffer -> view-space z at half resolution (level 0 of the z pyramid).
// Each texel takes the same pixel of its 2x2 footprint: always a surface of the scene (an average across a
// silhouette would not be), and unlike the closest of the four, it does not grow the foreground objects.

uniform sampler2D source_depth;
uniform vec2 depth_to_z; // orthographic projection: z_view = depth_to_z.x * ndc_z + depth_to_z.y

out float view_z;

const float BACKGROUND_Z = -1e6; // where nothing was drawn: far enough to never occlude

void main() {
    ivec2 p = min(2 * ivec2(gl_FragCoord.xy), textureSize(source_depth, 0) - 1);
    float depth = texelFetch(source_depth, p, 0).r;
    view_z = depth >= 1. ? BACKGROUND_Z : depth_to_z.x * (2. * depth - 1.) + depth_to_z.y;
}
