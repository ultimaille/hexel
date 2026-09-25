#version 330 core

in vec3 FragPos;
in vec3 Normal;
in float Value;

out vec4 FragColor;

uniform vec3 light_direction;
uniform vec3 color;
uniform sampler1D colormap;
uniform float ambient_prop;
uniform float color_map_prop;

void main()
{
    if(Value==-1) discard;


    //FragColor = vec4(texture(colormap, Value).rgb,1.);return;
    vec4 blend_color = color_map_prop * vec4(texture(colormap, Value).rgb,1.) + (1.-color_map_prop)*vec4(color,1.);
    vec3 N = normalize(Normal);
    vec3 L = light_direction;
    float coeff = ambient_prop+(1.-ambient_prop)*max(dot(N, L), 0.0);
    FragColor = coeff*blend_color;//vec4(coeff * color, 1.0);
}