#version 330 core

in vec2 TexCoord;

uniform sampler2D source_color;

out vec4 FragColor;

void main() {
    vec3 source = texture(source_color, TexCoord).rgb;

    // Perceived brightness of the original pixel.
    float luminance = dot(
        source,
        vec3(0.2126, 0.7152, 0.0722)
    );

    // Dark and light rose colors.
    vec3 dark_rose  = vec3(0.12, 0.005, 0.025);
    vec3 light_rose = vec3(1.00, 0.22, 0.42);

    vec3 rose = mix(
        dark_rose,
        light_rose,
        smoothstep(0.0, 1.0, luminance)
    );

    // Blend rather than completely replacing the source image.
    vec3 final_color = mix(source, rose, 0.15);

    FragColor = vec4(final_color, 1.0);
}

