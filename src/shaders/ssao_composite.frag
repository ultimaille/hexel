#version 330 core

in vec2 TexCoord;

uniform sampler2D source_color;
uniform sampler2D source_layer;
uniform sampler2D source_primitive;
uniform sampler2D source_ao;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragLayerIdColor;
layout(location = 2) out vec4 FragPrimitiveIdColor;

void main() {
    vec4 source = texture(source_color, TexCoord);
    float ao    = texture(source_ao,    TexCoord).r;
    FragColor = vec4(source.rgb * ao, source.a);
    FragLayerIdColor = texture(source_layer, TexCoord);
    FragPrimitiveIdColor = texture(source_primitive, TexCoord);
}

