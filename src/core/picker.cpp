#include "core.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"
#include "mode.h"

Picker::Picker(vec4 rect) : rect(rect) {
    reset();
}

Picker::Picker() : Picker(vec4{0, 0, 
                        static_cast<double>(God::context.render_target.width), 
                        static_cast<double>(God::context.render_target.height)}) {
    reset();
}

void Picker::reset() {
    God::context.begin_frame(true);
    God::context.begin_scissor(rect);
    God::layers.render();
    God::context.end_scissor();
    God::context.end_frame(true);
    God::context.render_target.read_framebuffer(layer_ids, rect, 1);
    God::context.render_target.read_framebuffer(vertex_ids, rect, 2);
    God::context.render_target.read_depth(depths, rect);
}

Picker::PickResult Picker::at(vec2 uv) {
    const int x = static_cast<int>(uv.x - rect[0]);
    const int y = static_cast<int>(uv.y - rect[1]);
    const int w = static_cast<int>(rect[2]);
    const int h = static_cast<int>(rect[3]);
    // Check limits to avoid out of image bound crash
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return {x, y ,-1, -1, ObjectId(), -1};
    }

    // Invert Y axis : OpenGL origin is on bottom-left
    int flipped_y = h - 1 - y;

    int off = (flipped_y * w + x) * 4;
    int layer_id = decode(layer_ids, off);
    int vertex_id = decode(vertex_ids, off);
    float depth = depths[flipped_y * w + x];
    auto layer_opt = God::layers.find_by_id(layer_id);

    // Retrieve primitive id from vertex id
    int primitive_id = -1;
    if (layer_opt.has_value()) {
        auto &layer = layer_opt.value().get();
        return {x, y, layer_id, layer.primitive_id(vertex_id), layer.mesh(), depth};
    }

    return {x, y, -1, -1, ObjectId(), depth};
}

vec3 Picker::PickResult::point() {
    float depth_ndc = depth * 2.f - 1.f; // Convert to NDC range [-1, 1]

    // Screen coordinates to NDC
    vec2 ndc = God::context.render_target.get_ndc(x, y);

    // Clip space coordinates
    vec4 clip_space{ndc.x, ndc.y, depth_ndc, 1.0f};

    // Unproject clip space to view space
    vec4 view_space = God::camera.projection_matrix().invert() * clip_space;

    // Unproject view space to world space
    mat4x4 inv_view = God::camera.view_matrix().invert();
    vec4 world_space = inv_view * (view_space / view_space.data[3]);
    return world_space.xyz();
}