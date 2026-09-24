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




//    // ------------------------------------------------------------
//    // Intersection rayon / sphère
//    // ------------------------------------------------------------
//
//    vec3 CO = O-C;
//    float a = dot(v,v); // =1
//    float b = 2.* dot(CO, v);
//    float c = dot(CO, CO) - R*R;
// 
//    float delta = b * b - 4.*a*c;
//
//    if (delta < 0.0) discard;
//
//    // Intersection la plus proche de la caméra
//    float sqrt_delta = sqrt(delta);
//
//    float t = (-b - sqrt_delta)/(2.*a);
//    if (t < 0.0) t =(-b + sqrt_delta)/(2.*a);
//    if (t < 0.0)   discard;
//
//
//    // ------------------------------------------------------------
//    // Position réelle sur la sphère
//    // ------------------------------------------------------------
//
//    vec3 I = O + t * v;
//
    // ------------------------------------------------------------
    // Intersection rayon / sphère
    // ------------------------------------------------------------

    vec3 P = O+dot(v , C-O)*v;

    float cp2 = dot(C-P,C-P);
    if (cp2>R*R) discard;

    vec3 I = P-sqrt(R*R-cp2)*v;

    //vec3 I = P;




    // ------------------------------------------------------------
    // Normale réelle
    // ------------------------------------------------------------

//    float d = 100.*sqrt(dot(I - C,I - C));//-R*R;
//    FragColor = vec4(d,d,d,1.0);
//    return;        
//
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
//    // ------------------------------------------------------------
//    // Intersection rayon / sphère
//    // ------------------------------------------------------------
//
//    vec3 oc = C-O;
//
//
//
//
//    float b = -dot(oc, dir);
//    // b est negatif
//    //FragColor =vec4( b,0.1*b,.01*b, 1.0 );return;
//
//    float c2 =
//        dot(oc, oc) -
//        R * R;
//
//    float discriminant =
//        b * b - c2;
//
//    if (discriminant < 0.0)
//        discard;
//
//
//    // Intersection la plus proche de la caméra
//    float sqrtDiscriminant =
//        sqrt(discriminant);
//
//    float t =
//        -b - sqrtDiscriminant;
//
//    if (t < 0.0)
//    {
//        t =
//            -b + sqrtDiscriminant;
//    }
//
//    if (t < 0.0)
//        discard;
//
//
//    // ------------------------------------------------------------
//    // Position réelle sur la sphère
//    // ------------------------------------------------------------
//
//    vec3 spherePosition = O + t * dir;
//
//
//    // ------------------------------------------------------------
//    // Normale réelle
//    // ------------------------------------------------------------
//
//    vec3 normal = normalize( spherePosition - C );
//
//
//    // ------------------------------------------------------------
//    // Vraie profondeur de la sphère
//    // ------------------------------------------------------------
//
//    vec4 clipPosition =
//        projection *
//        vec4(spherePosition, 1.0);
//
//    float ndcDepth =
//        clipPosition.z /
//        clipPosition.w;
//
//    gl_FragDepth =
//        ndcDepth * 0.5 + 0.5;
//
//
//    // ------------------------------------------------------------
//    // Éclairage
//    // ------------------------------------------------------------
//
//    //vec3 lightDirection =normalize(vec3(-1.0, -1.0, 1.0));
//
//    float diffuse =
//        max(
//            dot(normal, light_direction),
//            0.0
//        );
//
//    float lighting =
//        0.25 + 0.75 * diffuse;
//
//
//    // ------------------------------------------------------------
//    // Couleur
//    // ------------------------------------------------------------
//
//    FragColor =
//        vec4(
//            color * lighting,
//            1.0
//        );
//}