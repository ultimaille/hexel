#version 330 core

in vec3 C; // centre de la sphere

out vec4 FragColor;

uniform mat4 projection;
uniform mat4 inv_projection;

uniform float R;

uniform vec2 viewport;

uniform vec3 color;
uniform vec3 light_direction;

void main()
{
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

    vec4 clipPosition =
        projection *
        vec4(I, 1.0);

    float ndcDepth =
        clipPosition.z /
        clipPosition.w;

    gl_FragDepth =
        ndcDepth * 0.5 + 0.5;


    // ------------------------------------------------------------
    // Éclairage
    // ------------------------------------------------------------

    //vec3 lightDirection =normalize(vec3(-1.0, -1.0, 1.0));

    float diffuse =
        max(
            dot(normal, light_direction),
            0.0
        );

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
        }
