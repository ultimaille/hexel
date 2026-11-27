#version 330 core

in vec2 TexCoord;

uniform sampler2D source_ao;
uniform sampler2D source_depth; // scene depth
uniform vec2 texel_step;        // one texel along the blur direction, in uv units
uniform int blur_radius;

out vec4 FragColor;

const float THRESHOLD = 0.005;

float gaussian(float x, float sigma) {
    return exp(-(x * x) / (2.0 * sigma * sigma));
}

float get_z_coeff(vec2 uv) {
    float z = texture(source_depth, uv).r;
    return 3.0 * clamp(z - 0.1, 0.0, 1.0);
}

void main() {
    const float sigma = 2.0;
    float center_z = get_z_coeff(TexCoord);
    float weight_sum = 0.0;
    float ao_sum = 0.0;

    // single pass: accumulate the weights and the weighted values together, normalize at the end
    for (int i = -blur_radius; i <= blur_radius; ++i) {
        vec2 uv = TexCoord + texel_step * float(i);
        if (abs(get_z_coeff(uv) - center_z) <= THRESHOLD) {
            float w = gaussian(float(i), sigma);
            weight_sum += w;
            ao_sum += w * texture(source_ao, uv).r;
        }
    }

    // the center tap always passes the depth test, so weight_sum > 0
    FragColor = vec4(ao_sum / weight_sum);
}
