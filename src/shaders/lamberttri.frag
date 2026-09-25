#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    float diffuse = max(dot(N, L), 0.0);
    vec3 ambient = 0.55 * objectColor;
    vec3 diffuseColor = diffuse * lightColor * objectColor;
    vec3 color = ambient + diffuseColor;
    FragColor = vec4(color, 1.0);
//  FragColor = vec4(1.0);
}
