
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

uniform int layer_id;
flat out int frag_layer_id;
flat out int frag_primitive_id;

void main(){
    vec4 viewPos = view * vec4(aPos, 1.0);
    FragPos = viewPos.xyz;
    Normal = //mat3(transpose(inverse(view))) * 
    aNormal;
    gl_Position = projection * viewPos;
    
    frag_primitive_id = gl_VertexID / 3;
    frag_layer_id = layer_id;
}
