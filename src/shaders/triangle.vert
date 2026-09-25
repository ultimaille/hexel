#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in float value;


uniform mat4 view;
uniform mat4 projection;
uniform float texture_repeat;

out vec3 FragPos;
out vec3 Normal;
out float Value;

void main(){
    if (value==-1)  Value = -1;
    else  Value  = value * texture_repeat;
    vec4 viewPos = view * vec4(aPos, 1.0);
    FragPos = viewPos.xyz;
    Normal = mat3(transpose(inverse(view))) * aNormal;
    gl_Position = projection * viewPos;
}
 