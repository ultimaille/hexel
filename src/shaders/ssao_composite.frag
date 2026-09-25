#version 330 core

in vec2 TexCoord;

uniform sampler2D source_color;
uniform sampler2D source_ao;

out vec4 FragColor;

void main() {
    vec4 source = texture(source_color, TexCoord);
    float ao    = texture(source_ao,    TexCoord).r;
    FragColor = vec4(source.rgb * ao, source.a);
}

