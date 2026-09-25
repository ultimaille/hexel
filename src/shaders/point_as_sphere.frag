#version 330 core

in vec3 C; // centre de la sphere
in float Value;

out vec4 FragColor;

uniform mat4 projection;
uniform mat4 inv_projection;


uniform vec3 light_direction;
uniform vec3 color;
uniform sampler1D colormap;
uniform float ambient_prop;
uniform float color_map_prop;


uniform float R;
uniform vec2 viewport;


void main()
{
    if(Value==-1) 
        discard;

    // ------------------------------------------------------------
    // Coordonnées écran -> NDC
    // ------------------------------------------------------------

    vec2 ndc;
    ndc.x = (gl_FragCoord.x / viewport.x) * 2.0 - 1.0;
    ndc.y = (gl_FragCoord.y / viewport.y) * 2.0 - 1.0;


    // ------------------------------------------------------------
    // Rayon caméra passant par le fragment
    // ------------------------------------------------------------

    vec4 nearPoint = inv_projection * vec4(ndc, -1.0, 1.0);
    nearPoint.xyz /= nearPoint.w;

    vec4 farPoint = inv_projection * vec4(ndc, 1.0, 1.0);
    farPoint.xyz /= farPoint.w;

    vec3 O = nearPoint.xyz;
    vec3 v = normalize(farPoint.xyz - nearPoint.xyz);



    // ------------------------------------------------------------
    // Intersection rayon / sphère
    // ------------------------------------------------------------

    vec3 P = O+dot(v , C-O)*v;
    float cp2 = dot(C-P,C-P);
    if (cp2>R*R) discard;
    vec3 I = P-sqrt(R*R-cp2)*v;


    // ------------------------------------------------------------
    // Normale réelle
    // ------------------------------------------------------------
    vec3 normal = normalize( I - C );


    // ------------------------------------------------------------
    // Vraie profondeur de la sphère
    // ------------------------------------------------------------

    vec4 clipPosition =projection *vec4(I, 1.0);
    float ndcDepth = clipPosition.z / clipPosition.w;
    gl_FragDepth = ndcDepth * 0.5 + 0.5;


    // ------------------------------------------------------------
    // Éclairage
    // ------------------------------------------------------------


    float diffuse =max(dot(normal, light_direction),0.0);
    vec4 blend_color = color_map_prop * vec4(texture(colormap, Value).rgb,1.) + (1.-color_map_prop)*vec4(color,1.);
    float coeff = ambient_prop+(1.-ambient_prop)*diffuse;
    FragColor = coeff*blend_color;

}
