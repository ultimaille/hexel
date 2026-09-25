#version 330 core

in vec2 TexCoord;

uniform sampler2D source_ao;
uniform vec2 blur_direction;
uniform float texel_size;
uniform int blur_radius;

out vec4 FragColor;

float gaussian_weight(float x, float sigma) {
    return exp(-0.5 * (x * x) / (sigma * sigma));
}

void main() {
    float total = 0.0;
    float weight_total = 0.0;
    float sigma = 2.0;
    for (int i = -8; i <= 8; ++i) {
        if (abs(i) > blur_radius) {
            continue;
        }

        float weight = gaussian_weight(float(i), sigma);
        vec2 uv = TexCoord + blur_direction * float(i) * texel_size;
        total += texture(source_ao, uv).r * weight;
        weight_total += weight;
    }

    float ao = total / weight_total;
    FragColor = vec4(ao, ao, ao, 1.0);
}

