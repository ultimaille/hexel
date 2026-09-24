#version 330 core

in vec2 TexCoord;

uniform sampler2D source_color;
uniform sampler2D source_depth;
uniform mat4 projection;
uniform mat4 inverse_projection;

out vec4 FragColor;

vec3 reconstruct_view_position(vec2 uv, float depth) {
    vec4 clip_position = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view_position = inverse_projection * clip_position;
    return view_position.xyz / view_position.w;
}

void main() {
    float depth = texture(source_depth, TexCoord).r;
    vec3 position = reconstruct_view_position(TexCoord, depth);
    float visualization = clamp((position.z + 1.0) / 2.0, 0.0, 1.0);

    FragColor = vec4(
        visualization,
        visualization,
        visualization,
        1.0
    );
}
