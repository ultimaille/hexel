#version 330 core

in vec3 FragPos;
in vec3 Normal;

// outputs
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragLayerIdColor;
layout(location = 2) out vec4 FragPrimitiveIdColor;

flat in int frag_layer_id;
flat in int frag_primitive_id;

uniform vec3 light_direction;
uniform vec3 color;

vec3 encode_id(int id) {
    int r = id & 0x000000FF;
    int g = (id & 0x0000FF00) >> 8;
    int b = (id & 0x00FF0000) >> 16;
    return vec3(r / 255.f, g / 255.f, b / 255.f); 
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = light_direction;
    float coeff = .2+.8*max(dot(N, L), 0.0);
    FragColor = vec4(coeff * color, 1.0);
    FragLayerIdColor = vec4(encode_id(frag_layer_id), 1.);
    FragPrimitiveIdColor = vec4(encode_id(frag_primitive_id), 1.);
}
