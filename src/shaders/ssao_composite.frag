#version 330 core

// Output the ambient occlusion factor; the blend state multiplies it into the target's color.

in vec2 TexCoord;

uniform sampler2D source_ao;

out vec4 FragColor;

void main() {
    FragColor = vec4(texture(source_ao, TexCoord).r);
}
