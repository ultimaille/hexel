#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 light_direction;
uniform vec3 color;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = light_direction;
    float coeff = .2+.8*max(dot(N, L), 0.0);
    FragColor = vec4(coeff * color, 1.0);
}