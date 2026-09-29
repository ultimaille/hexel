#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float value;

uniform mat4 view;
uniform mat4 projection;
uniform float texture_repeat;
uniform vec2 data_range;

out vec3 FragPos;
out vec3 Normal;
out float Value;

uniform int layer_id;
// flat out int frag_layer_id;
// flat out int frag_primitive_id;

void main(){
    if (value==-1)  Value = -1;
    else  {
        Value = (value - data_range.x) / (data_range.y - data_range.x);
        Value = Value * texture_repeat;
    }

    vec4 viewPos = view * vec4(aPos, 1.0);
    FragPos = viewPos.xyz;
    Normal = normalize(mat3(transpose(inverse(view))) * aNormal);
    gl_Position = projection * viewPos;
    // frag_primitive_id = gl_VertexID / 3;
    // frag_layer_id = layer_id;
}
 