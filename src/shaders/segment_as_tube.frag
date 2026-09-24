#version 330 core

flat in vec3 vP0;
flat in vec3 vP1;

out vec4 FragColor;

uniform mat4 projection;
uniform mat4 inv_projection;

uniform vec2 viewport;

uniform float R;

uniform vec3 color;

void main()
{
    //FragColor =vec4(color,1.0); return;
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

    vec3 axis = vP1 - vP0;
    
    float f = 100.*cross(normalize(vP1 - vP0),O-vP0).z;
    FragColor =vec4(0,f,1.-f,1.0); return;

    float lengthAxis = length(axis);

    if (lengthAxis < 0.000001)
        discard;

    vec3 w = axis / lengthAxis;


    // ============================================================
    // 3. Intersection rayon / cylindre infini
    //
    // On retire la composante parallèle à l'axe.
    // ============================================================

    vec3 delta =O - vP0;

    vec3 dPerp =v -dot(v, w) * w;

    vec3 deltaPerp =delta -dot(delta, w) * w;


    float A =dot(dPerp, dPerp);

    float B =2.0 * dot(dPerp, deltaPerp);

    float C =dot(deltaPerp, deltaPerp)- R * R;


    float discriminant = B * B - 4.0 * A * C;


    float bestT = 1e30;
    bool hit = false;


    // ============================================================
    // 4. Intersection avec la surface latérale
    // ============================================================

    if (A > 0.0000001 && discriminant >= 0.0)
    {
        float sqrtD = sqrt(discriminant);

        float t0 = (-B - sqrtD) / (2.0 * A);

        float t1 = (-B + sqrtD) / (2.0 * A);


        // Première intersection
        if (t0 >= 0.0)
        {
            vec3 hitPosition =
                O +
                t0 * v;

            float axial =
                dot(
                    hitPosition - vP0,
                    w
                );

            if (axial >= 0.0 &&
                axial <= lengthAxis)
            {
                bestT = t0;
                hit = true;
            }
        }


        // Deuxième intersection
        if (t1 >= 0.0 && t1 < bestT)
        {
            vec3 hitPosition =
                O +
                t1 * v;

            float axial =
                dot(
                    hitPosition - vP0,
                    w
                );

            if (axial >= 0.0 &&
                axial <= lengthAxis)
            {
                bestT = t1;
                hit = true;
            }
        }
    }


    // ============================================================
    // 5. Caps du cylindre
    // ============================================================

    // ============================================================
    // 6. Pas d'intersection
    // ============================================================

    if (!hit)
        discard;
    FragColor =vec4(color,1.0); return;






















    // ============================================================
    // 7. Position réelle sur le cylindre
    // ============================================================

    vec3 hitPosition =
        O +
        bestT * v;


    // ============================================================
    // 8. Normale
    // ============================================================

    float axial =
        dot(
            hitPosition - vP0,
            w
        );


    vec3 normal;

    if (axial <= 0.0001)
    {
        // Cap P0
        normal = -w;
    }
    else if (axial >= lengthAxis - 0.0001)
    {
        // Cap P1
        normal = w;
    }
    else
    {
        // Surface latérale
        vec3 radial =
            hitPosition - vP0;

        radial -=
            dot(radial, w) * w;

        normal =
            normalize(radial);
    }


    // ============================================================
    // 9. Vraie profondeur
    // ============================================================

    vec4 clipPosition =
        projection *
        vec4(hitPosition, 1.0);

    float ndcDepth =
        clipPosition.z /
        clipPosition.w;

    gl_FragDepth =
        ndcDepth * 0.5 + 0.5;


    // ============================================================
    // 10. Éclairage
    // ============================================================

    vec3 lightDirection =
        normalize(vec3(-1.0, -1.0, 1.0));

    float diffuse =
        max(
            dot(normal, lightDirection),
            0.0
        );

    float lighting =
        0.25 + 0.75 * diffuse;


    FragColor =
        vec4(
            color * lighting,
            1.0
        );
}