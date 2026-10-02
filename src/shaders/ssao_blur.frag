#version 330 core

in vec2 TexCoord;

uniform sampler2D source_ao;
uniform sampler2D source_depth;
uniform vec2 blur_direction;
uniform float texel_size;
uniform int blur_radius;

out vec4 FragColor;

const float PI = 3.14159265;
const float THRESHOLD = 0.005;

float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

float get_z_coeff(vec2 uv) {
    float z = texture(source_depth, uv).r;
    return 3.0 * clamp(z - 0.1, 0.0, 1.0);
}

float get_z_dist(vec2 a, vec2 b) {
    return abs(get_z_coeff(a) - get_z_coeff(b));
}

void main() {
    float sigma = 2.0;
    float sum = 0.0;
    float ao_sum = 0.0;

    vec2 offset = blur_direction * texel_size;

    // first pass: compute valid weight sum
    for (int i = -blur_radius; i <= blur_radius; ++i) {
        vec2 uv = TexCoord + offset * float(i);
        if (get_z_dist(TexCoord, uv) <= THRESHOLD) {
            float w = gaussian(float(i), sigma);
            sum += w;
        }
    }

    if (sum <= 0.0) {
        FragColor = vec4(texture(source_ao, TexCoord).r, texture(source_ao, TexCoord).r, texture(source_ao, TexCoord).r, 1.0);
        return;
    }

    for (int i = -blur_radius; i <= blur_radius; ++i) {
        vec2 uv = TexCoord + offset * float(i);

        if (get_z_dist(TexCoord, uv) <= THRESHOLD) {
            float w = gaussian(float(i), sigma);
            float ao = texture(source_ao, uv).r;
            ao_sum += ao * (w / sum);
        }
    }

    FragColor = vec4(ao_sum, ao_sum, ao_sum, 1.0);
}

