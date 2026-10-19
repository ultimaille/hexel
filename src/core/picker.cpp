#include "core.h"
#include "event.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"
#include "mode.h"

Picker::Picker() : 
    w(God::context.render_target.width), 
    h(God::context.render_target.height) {
    reset();
}

void Picker::reset() {
    data.clear();
    
    for (auto group_id : God::context.render_target.buffer_groups()) {
        BufferData r;
        r.w = w;
        r.h = h;
        r.group_id = group_id;
        God::context.render_target.read_framebuffer(r.layer_ids, 0, 0, w, h, 1, group_id);
        God::context.render_target.read_framebuffer(r.primitive_ids, 0, 0, w, h, 2, group_id);
        God::context.render_target.read_depth(r.depths, 0, 0, w, h, group_id);
        data.emplace(group_id, r);
    }
}

Picker::Result Picker::at(vec2 uv, int buffer_group_id) {
    return data[buffer_group_id].at(uv);
}


Picker::Result Picker::BufferData::at(vec2 uv) {
    int x = static_cast<int>(uv.x);
    int y = static_cast<int>(uv.y);
    // Check limits to avoid out of image bound crash
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return {};
    }

    // Invert Y axis : OpenGL origin is on bottom-left
    int flipped_y = h - 1 - y;

    int off = (flipped_y * w + x) * 4;

    Result results;
    int layer_id = decode(layer_ids, off);
    int primitive_id = decode(primitive_ids, off);
    float depth = depths[flipped_y * w + x];

    auto layer_opt = God::layers.find_by_id(layer_id);

    // Retrieve primitive id from vertex id
    //int primitive_id = -1;
    if (layer_opt.has_value()) {
        auto &layer = layer_opt.value().get();
        return {x, y, layer_id, primitive_id, layer.mesh, depth};
    }
    
    return {x, y, -1, -1, ObjectId(), depth};
}

vec3 Picker::Result::point() {
    // float depth_ndc = depth * 2.f - 1.f; // Convert to NDC range [-1, 1]

    // // Screen coordinates to NDC
    // vec2 ndc = God::context.render_target.get_ndc(x, y);

    // // Clip space coordinates
    // vec4 clip_space{ndc.x, ndc.y, depth_ndc, 1.0f};

    // // Unproject clip space to view space
    // vec4 view_space = God::camera.projection_matrix().invert() * clip_space;

    // // Unproject view space to world space
    // mat4x4 inv_view = God::camera.view_matrix().invert();
    // vec4 world_space = inv_view * (view_space / view_space.data[3]);
    // return world_space.xyz();
    return vec3{0,0,0};
}

int Picker::Result::filtered_primitive_id(ObjectId& requested_mesh, int requested_primitive_type){
    FOR(l, God::layers.size()) {
        if (God::layers[l].id != layer_id)                    continue;
        if (God::layers[l].mesh != requested_mesh)            continue;
        // we directly have the right primitive
        int picked_type = God::layers[l].primitive_type();
        if (picked_type == requested_primitive_type)   return  raw_primitive_id;
        // the requested primitive can be deduced from the picked primitive
        if (requested_mesh.path == events::ObjectType::HEXAHEDRA && picked_type == 2 && requested_primitive_type == 1)// convert face to cell
            return  raw_primitive_id / 6;
        if (requested_mesh.path == events::ObjectType::HEXAHEDRA && picked_type == 3 && requested_primitive_type == 1)// convert corner to cell
            return  raw_primitive_id / 8;
    }
    return -1;
}
