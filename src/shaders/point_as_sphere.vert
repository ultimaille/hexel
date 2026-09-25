#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in float value;

uniform mat4 view;
uniform mat4 projection;
uniform float texture_repeat;

uniform float R;

// Taille du viewport en pixels
uniform vec2 viewport;

// center of the sphere in view space
out vec3 C;
out float Value;

void main(){
    if (value==-1)  Value = -1;
    else  Value  = value ;//texture_repeat;
    // ------------------------------------------------------------
    // Centre de la sphère en espace caméra
    // ------------------------------------------------------------

    vec4 centerView = view * vec4(aPosition, 1.0);

    C = centerView.xyz;


    // ------------------------------------------------------------
    // Position du centre
    // ------------------------------------------------------------

    gl_Position = projection * centerView;


    // ------------------------------------------------------------
    // Taille de la sphère en pixels
    // ------------------------------------------------------------

    float diameterPixels =
        2.*R
        * projection[1][1]
        * viewport.y
        ;

    gl_PointSize = diameterPixels;
}