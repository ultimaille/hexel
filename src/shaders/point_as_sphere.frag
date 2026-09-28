#version 330 core

in vec3 C; // centre de la sphere
in float Value;
// Color output
layout(location = 0) out vec4 FragColor;
layout(location = 2) out vec4 FragVertexIndexColor;

flat in int fragVertexIndex;

uniform mat4 projection;
uniform mat4 inv_projection;
uniform vec3 light_direction;
uniform vec3 color;
uniform sampler1D colormap;
uniform float ambient_prop;
uniform float color_map_prop;
uniform float R;
uniform vec2 viewport;


vec3 encode_id(int id) {
    int r = id & 0x000000FF;
    int g = (id & 0x0000FF00) >> 8;
    int b = (id & 0x00FF0000) >> 16;
    return vec3(r / 255.f, g / 255.f, b / 255.f); 
}

void main()
{
    if(Value==-1) 
        discard;

    vec2 ndc;
    ndc.x = (gl_FragCoord.x / viewport.x) * 2.0 - 1.0;
    ndc.y = (gl_FragCoord.y / viewport.y) * 2.0 - 1.0;


    // rayon passant par le pixel=> O en direction v
    vec4 nearPoint = inv_projection * vec4(ndc, -1.0, 1.0);
    vec3 O = nearPoint.xyz / nearPoint.w;
    vec3 v = vec3(0,0,-1);

    // intersection rayon/sphere I
    vec3 P = O+dot(v , C-O)*v;
    float cp2 = dot(C-P,C-P);
    if (cp2>R*R) discard;
    vec3 I = P-sqrt(R*R-cp2)*v;


    // update the depth
    vec4 clipPosition =projection *vec4(I, 1.0);
    gl_FragDepth = clipPosition.z / clipPosition.w * 0.5 + 0.5;



    // compute lighting
    float diffuse =max(dot(normalize( I - C ), light_direction),0.0);
    vec4 blend_color = color_map_prop * vec4(texture(colormap, Value).rgb,1.) + (1.-color_map_prop)*vec4(color,1.);
    float coeff = ambient_prop+(1.-ambient_prop)*diffuse;
    FragColor = coeff*blend_color;

    float lighting =
        0.25 + 0.75 * diffuse;


    // ------------------------------------------------------------
    // Couleur
    // ------------------------------------------------------------

    FragColor =
        vec4(
            color * lighting,
            1.0
        );

    FragVertexIndexColor = vec4(encode_id(fragVertexIndex), 1.);
}
