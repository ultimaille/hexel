#version 330 core

in vec2 TexCoord;

uniform sampler2D source_depth;
uniform sampler2D random_texture;
// orthographic projection (TrackBallCamera): view-space coordinates are affine in uv and depth
uniform vec2 depth_to_z;    // z_view  = depth_to_z.x * ndc_z + depth_to_z.y
uniform vec4 uv_to_view;    // xy_view = uv * uv_to_view.xy + uv_to_view.zw

uniform float max_radius;
uniform float step_mul;

out vec4 FragColor;

float width  = float(textureSize(source_depth,0).x);
float height = float(textureSize(source_depth,0).y);

const float PI = 3.14159265359;
const float BACKGROUND_Z = -10000.; // returned for pixels where nothing was drawn

float get_obj_z(in vec2 uv) {
    float depth = texture(source_depth, uv).r;
    return depth >= 1. ? BACKGROUND_Z : depth_to_z.x * (2. * depth - 1.) + depth_to_z.y;
}

vec3 get_obj_coords(in vec2 uv) {
    return vec3(uv * uv_to_view.xy + uv_to_view.zw, get_obj_z(uv));
}

bool outside(vec2 uv) {
    return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

// returns false when no sample was found (e.g. everything along dir is background)
bool horizon_point(in vec2 from, in vec2 dir, out vec2 result) {
    bool found = false;
    float horizon_delta = -1e30;
    float from_z = get_obj_z(from);
    float step = (1.0 / width);
    float r = 2.0 * step;
    vec2 cur_point = from + r * dir;

    while (!outside(cur_point)) {
        float z = get_obj_z(cur_point);

        float delta_z = (z - from_z) / r;
        if (delta_z > horizon_delta) {
            horizon_delta = delta_z;
            result = cur_point;
            found = true;
        }
        if (r > max_radius) {
            break;
        }
        r += step;
        step *= step_mul;
        cur_point = from + r * dir;
    }
    return found;
}

float horizon_angle(in vec2 from, in vec3 from3D, in vec2 dir, in vec3 normal) {
    vec2 h;
    if (!horizon_point(from, dir, h))
        return PI / 2.0; // flat horizon: no occlusion
    vec3 horizon = get_obj_coords(h) - from3D;
    return acos ( dot(normal, horizon) / length(horizon) );
}

float my_noise() {
    vec2 random_size = vec2(textureSize(random_texture, 0));
    vec2 random_uv = TexCoord * vec2(width, height) / random_size;
    return texture(random_texture, fract(random_uv)).r;
}

float ambient_occlusion(in vec2 from) {
    const int nb_directions = 7;
    float angle_step = 2.0 * PI / (nb_directions);
    float cur_angle = my_noise() * 2. * PI ;
    float occlusion_factor = 0.0;
    vec3 from3D = get_obj_coords(from);
    for (int i=0; i < nb_directions; i++) {
        vec2 dir = vec2(cos(cur_angle), sin(cur_angle));
        float h_angle = horizon_angle(from, from3D, dir, vec3(0., 0., 1.));
        cur_angle += angle_step;
        occlusion_factor += h_angle;
    }
    return occlusion_factor / (float(nb_directions) * (PI / 2.0));
}

void main() {
    if (get_obj_z(TexCoord) <= BACKGROUND_Z) {
        FragColor = vec4(1.0);
        return;
    }
    float g = ambient_occlusion(TexCoord);
    FragColor = vec4(vec3(g), 1.0);
}

