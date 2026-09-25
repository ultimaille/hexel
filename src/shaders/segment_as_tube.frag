#version 330 core

flat in vec3 A;
flat in vec3 B;
in float value;

out vec4 FragColor;

uniform mat4 projection;
uniform mat4 inv_projection;

uniform vec2 viewport;
uniform float R_dest;
uniform float R_org;

uniform vec3 light_direction;
uniform vec3 color;
uniform sampler1D colormap;
uniform float ambient_prop;
uniform float color_map_prop;


void main()
{
    if(value==-1) discard;

    // ============================================================
    // 1. Rayon caméra passant par le fragment
    // ============================================================

    vec2 ndc;
    ndc.x = 2.0 * gl_FragCoord.x / viewport.x - 1.0;
    ndc.y = 2.0 * gl_FragCoord.y / viewport.y - 1.0;


    vec4 nearPoint = inv_projection * vec4(ndc, -1.0, 1.0);
    nearPoint.xyz /= nearPoint.w;
    vec3 O = nearPoint.xyz;

    vec3 v = vec3(0,0,-1);


    // ============================================================
    // 2. Axe du cylindre
    // ============================================================
    vec3 axis = B - A;



    float lengthAxis = length(axis);
    if (lengthAxis < 0.000001) discard;

    vec3 w = axis / lengthAxis;

    float r = .5*cross(w,O-A).z;


    float linearDepth = -(gl_FragCoord.z * 2.0 - 1.0 - projection[3][2]) / projection[2][2];

    vec3 I = vec3(O.xy,-linearDepth);
    float bary =abs(dot(I-A,axis))/dot(axis,axis);  
    float R = bary * R_org + (1.-bary) * R_dest;
    if (r>R) discard;


    
    float d = sqrt(max(0,R*R-r*r));
    linearDepth -= d /length(w.xy);
    
    gl_FragDepth = .5*(-projection[2][2] * linearDepth  + 1.0 + projection[3][2]);

    vec3 hitPosition =vec3(O.xy,-linearDepth);




    // ============================================================
    // 8. Normale
    // ============================================================

        vec3 radial =hitPosition - A;
        radial -=dot(radial, w) * w;
        vec3 normal =normalize(radial);



    // ============================================================
    // 10. Éclairage
    // ============================================================

    float diffuse =max(dot(normal, light_direction),0.0);
    vec4 blend_color = color_map_prop * vec4(texture(colormap, value).rgb,1.) + (1.-color_map_prop)*vec4(color,1.);
    float coeff = ambient_prop+(1.-ambient_prop)*diffuse;
    FragColor = coeff*blend_color;
}