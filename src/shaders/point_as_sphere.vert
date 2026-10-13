#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in float value;
layout(location = 2) in int visible;
layout (location = 3) in int primitive_id;

uniform mat4 view;
uniform mat4 projection;
uniform float texture_repeat;
uniform vec2 data_range;

uniform float R;
uniform vec2 viewport;

// center of the sphere in view space
out vec3 C;
out float Value;

uniform int layer_id;
flat out int frag_layer_id;
flat out int frag_primitive_id;
flat out int frag_visible;

void main(){
    Value = texture_repeat*(value - data_range.x) / (data_range.y - data_range.x);
    
    vec4 centerView = view * vec4(aPosition, 1.0);
    C = centerView.xyz;
    gl_Position = projection * centerView;
    gl_PointSize = 2.*R* projection[1][1]* viewport.y;
    frag_primitive_id = primitive_id;
    frag_layer_id = layer_id;
    frag_visible = visible;
}