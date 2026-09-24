#version 330 core

layout(location = 0) in vec3 aP0;
layout(location = 1) in vec3 aP1;

uniform mat4 view;
uniform mat4 projection;

flat out vec3 vP0;
flat out vec3 vP1;

void main()
{
    vec3 p0 =(view * vec4(aP0, 1.0)).xyz;

    vec3 p1 =(view * vec4(aP1, 1.0)).xyz;

    vP0 = p0;
    vP1 = p1;

    // Pour GL_LINES :
    //
    // sommet 0 -> P0
    // sommet 1 -> P1
    //
    if (gl_VertexID % 2 == 0)
    {
        gl_Position =
            projection *
            vec4(p0, 1.0);
    }
    else
    {
        gl_Position =
            projection *
            vec4(p1, 1.0);
    }
}