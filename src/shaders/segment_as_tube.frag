#version 330 core

flat in vec3 A;
flat in vec3 B;
in float value;

// outputs
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragLayerIdColor;
layout(location = 2) out vec4 FragPrimitiveIdColor;

flat in int frag_layer_id;
flat in int frag_primitive_id;
flat in int frag_visible;

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

vec3 encode_id(int id) {
    int r = id & 0x000000FF;
    int g = (id & 0x0000FF00) >> 8;
    int b = (id & 0x00FF0000) >> 16;
    return vec3(r / 255.f, g / 255.f, b / 255.f); 
}

void main(){
    if(frag_visible == 0) discard;

    // normalized pixel position
    vec2 ndc;
    ndc.x = 2.0 * gl_FragCoord.x / viewport.x - 1.0;
    ndc.y = 2.0 * gl_FragCoord.y / viewport.y - 1.0;

    // rayon passant par le pixel=> O en direction v
    vec4 nearPoint = inv_projection * vec4(ndc, -1.0, 1.0);
    vec3 O = nearPoint.xyz / nearPoint.w;
    vec3 v = vec3(0,0,-1);


    
    // cylinder axis AB and its normalized vector w
    vec3 AB = B - A;
    float lengthAB = length(AB);
    if (lengthAB < 0.000001) discard;
    vec3 w = AB / lengthAB;

    // distance of the fragment to the projection of AB on the screen plane
    float r = .5*cross(w,O-A).z;

    // intersection I of the fragment ray to the "square generated from AB by GL_LINES"
    float linearDepth = -(gl_FragCoord.z * 2.0 - 1.0 - projection[3][2]) / projection[2][2];
    vec3 I = vec3(O.xy,-linearDepth);
    

    // for a cylinder, this section should be: float R = R_org;
    float bary =abs(dot(I-A,AB))/dot(AB,AB);
    //float R = bary * R_org + (1.-bary) * R_dest; //==> code to have a kind of truncated cone
    float R = R_org;  if (bary>.8 && R_org!=R_dest) R = 5.*(1.-bary) * R_dest ;//==> code to generate a kind of arrow

    if (r>R) discard;

    // correct the depth to had the geometry of the cylinder (wrong but OK for trunkated cone or arrow) 
    float d = sqrt(max(0,R*R-r*r));
    linearDepth -= d /length(w.xy);
    
    // update the depth and get the ray cylinder intersection
    gl_FragDepth = .5*(-projection[2][2] * linearDepth  + 1.0 + projection[3][2]);
    vec3 hitPosition =vec3(O.xy,-linearDepth);

    // lighting
    vec3 radial =hitPosition - A;
    vec3 normal =normalize(radial -dot(radial, w) * w);
    float diffuse =max(dot(normal, light_direction),0.0);
    vec4 blend_color = color_map_prop * vec4(texture(colormap, value).rgb,1.) + (1.-color_map_prop)*vec4(color,1.);
    float coeff = ambient_prop+(1.-ambient_prop)*diffuse;
    FragColor = coeff*blend_color;
    FragLayerIdColor = vec4(encode_id(frag_layer_id), 1.);
    FragPrimitiveIdColor = vec4(encode_id(frag_primitive_id), 1.);
}