#version 330 core

layout(location = 0) in vec3 aP0;
layout(location = 1) in float ValueA;
layout(location = 2) in vec3 aP1;
layout(location = 3) in float ValueB;

uniform mat4 view;
uniform mat4 projection;
uniform float texture_repeat;

flat out vec3 A;
flat out vec3 B;
out float value;

void main()
{


    vec3 p0 =(view * vec4(aP0, 1.0)).xyz;
    vec3 p1 =(view * vec4(aP1, 1.0)).xyz;

    A = p0;
    B = p1;

    // Pour GL_LINES :
    //
    // sommet 0 -> P0
    // sommet 1 -> P1
    //
    if (gl_VertexID % 2 == 0){
        value = ValueA* texture_repeat;
        gl_Position =projection *vec4(p0, 1.0);
    }else{
        value = ValueB* texture_repeat;
        gl_Position =projection *vec4(p1, 1.0);
    }
    if (ValueA==-1)  value = -1;
    if (ValueB==-1)  value = -1;

}