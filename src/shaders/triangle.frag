#version 330 core

in vec3 FragPos;
in vec3 Normal;
in float Value;

// outputs
out vec4 FragColor;
layout(location = 1) out vec4 FragLayerIdColor;
layout(location = 2) out vec4 FragVertexIdColor;

flat in int frag_layer_id;
flat in int frag_vertex_id;

uniform vec3 light_direction;
uniform vec3 color;
uniform sampler1D colormap;
uniform float ambient_prop;
uniform float color_map_prop;

vec3 encode_id(int id) {
    int r = id & 0x000000FF;
    int g = (id & 0x0000FF00) >> 8;
    int b = (id & 0x00FF0000) >> 16;
    return vec3(r / 255.f, g / 255.f, b / 255.f); 
}


void main(){
    if(Value==-1) discard;
    vec4 blend_color = color_map_prop * vec4(texture(colormap, Value).rgb,1.) + (1.-color_map_prop)*vec4(color,1.);
    float coeff = ambient_prop+(1.-ambient_prop)*max(dot(Normal, light_direction), 0.0);
    FragColor = coeff*blend_color;
    FragLayerIdColor = vec4(encode_id(frag_layer_id), 1.);
    FragVertexIdColor = vec4(encode_id(frag_vertex_id), 1.);
}