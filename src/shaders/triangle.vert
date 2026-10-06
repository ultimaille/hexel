#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float value;
layout (location = 3) in vec3 bary;
layout (location = 4) in int visible;

// layout (location = 5) in int primitive_id;
// uniform usamplerBuffer ivisible;

uniform mat4 view;
uniform mat4 projection;
uniform float texture_repeat;
uniform vec2 data_range;

out vec3 FragPos;
out vec3 Normal;
out float Value;
out vec3 FragWorldPos;
out vec3 FragBary;

uniform int layer_id;
flat out int frag_layer_id;
flat out int frag_primitive_id;
flat out int frag_visible;


void main(){
    if (value==-1)  Value = -1;
    else  {
        Value = (value - data_range.x) / (data_range.y - data_range.x);
        Value = Value * texture_repeat;
    }

    vec3 p = aPos - (aPos - bary) * 1.;
    vec4 viewPos = view * vec4(aPos, 1.0);
    FragPos = viewPos.xyz;
    FragWorldPos = aPos;
    FragBary = bary;
    Normal = normalize(mat3(transpose(inverse(view))) * aNormal);
    gl_Position = projection * viewPos;
    frag_primitive_id = gl_VertexID / 3;
    frag_layer_id = layer_id;
    frag_visible = visible;
    // frag_visible = int(texelFetch(ivisible, primitive_id));
}
 