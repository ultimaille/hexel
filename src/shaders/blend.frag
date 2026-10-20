#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D tex_a;
uniform sampler2D tex_b;
uniform int mix;

void main()
{
    vec4 color_a = texture(tex_a, TexCoord);
    vec4 color_b = texture(tex_b, TexCoord);

    if (mix == 0) { // NONE
        FragColor = color_b;
    } else if (mix == 1) { // AVERAGE
        if (length(color_a.rgb) <= 0.01)
            FragColor = color_b;
        else if (length(color_b.rgb) <= 0.01)
            FragColor = color_a;
        else
            FragColor = (color_a + color_b) * .5;
    }
}