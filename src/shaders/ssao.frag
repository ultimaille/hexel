#version 330 core

in vec2 TexCoord;

uniform sampler2D source_color;
uniform sampler2D source_depth;
uniform sampler2D random_texture;
uniform mat4 inverse_projection;

uniform float max_radius;
uniform float step_mul;

out vec4 FragColor;

float width  = float(textureSize(source_depth,0).x);
float height = float(textureSize(source_depth,0).y);

const float PI = 3.14159265359;

vec3 get_obj_coords(in vec2 uv) {
    vec4 p = vec4(uv, texture(source_depth, uv).r, 1.);
    // Map [0,1] to [-1,1]
    p.xyz = p.xyz * 2. - 1.;
    p = inverse_projection * p;
    if (p.w != 0.) {
        p.xyz /= p.w;
    }
    return vec3(p);
}

float get_obj_z(in vec2 uv) {
    float depth = texture(source_depth, uv).r;
    // For background points, return infinite value.
    if (depth >= 1.) {
        return -10000.;
    }
    // Map from range 0 to 1 to range -1 to 1
    depth = depth * 2. - 1.;
    float z = (depth * inverse_projection[2][2] + inverse_projection[3][2]) / (depth * inverse_projection[2][3] + inverse_projection[3][3]);
    return z;
}

vec3 reconstruct_view_position(vec2 uv, float depth) {
    vec4 clip_position = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view_position = inverse_projection * clip_position;
    return view_position.xyz / view_position.w;
}

bool outside(vec2 uv) {
    return uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0;
}

vec3 read_position(vec2 uv) {
    return reconstruct_view_position(uv, get_obj_z(uv));
}

vec2 horizon_point(in vec2 from, in vec2 dir) {
    vec2 result;
    float horizon_delta = -100000.0;
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
        }
        if (r > max_radius) {
            break;
        }
        r += step;
        step *= step_mul;
        cur_point = from + r * dir;
    }

    return result;
}

float horizon_angle(in vec2 from, in vec3 from3D, in vec2 dir, in vec3 normal) {
    vec3 horizon = get_obj_coords(horizon_point(from, dir)) - from3D;
    return acos ( dot(normal, horizon) / length(horizon) );
}

float my_noise() {
    vec2 random_size = vec2(textureSize(random_texture, 0));
    vec2 random_uv = TexCoord * vec2(width, height) / random_size;
    return texture(random_texture, fract(random_uv)).r;
}

float ambient_occlusion(in vec2 from) {
    const int nb_directions = 7;
    vec2 directions[8] = vec2[](
            vec2( 1.0,  0.0),
            vec2(-1.0,  0.0),
            vec2( 0.0,  1.0),
            vec2( 0.0, -1.0),
            normalize(vec2( 1.0,  1.0)),
            normalize(vec2(-1.0,  1.0)),
            normalize(vec2( 1.0, -1.0)),
            normalize(vec2(-1.0, -1.0))
            );

    float angle_step = 2.0 * PI / (nb_directions);
    float cur_angle = my_noise() * 2. * PI ;
    float occlusion_factor = 0.0;
    vec3 from3D = get_obj_coords(from);
    for (int i=0; i < nb_directions; i++) {
        vec2 dir = vec2(cos(cur_angle), sin(cur_angle));
//        dir = directions[i];

        float h_angle = horizon_angle(from, from3D, dir, vec3(0., 0., 1.));
       cur_angle += angle_step;
        occlusion_factor += h_angle;
    }
    return occlusion_factor / (float(nb_directions) * (PI / 2.0));
}

void main() {
    if (texture(source_depth, TexCoord).r >= 1.0) {
        return;
    }
    float g = ambient_occlusion(TexCoord);
    FragColor = vec4(vec3(g), 1.0);
}

