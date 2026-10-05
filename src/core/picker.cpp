#include "core.h"

Picker::Picker(vec4 rect) : rect(rect) {
    God::context.render_target.read_framebuffer(layer_ids, rect, 1);
    God::context.render_target.read_framebuffer(vertex_ids, rect, 2);
}

Picker::Picker() : Picker(vec4{0, 0, 
                        static_cast<double>(God::context.render_target.width), 
                        static_cast<double>(God::context.render_target.height)}) {
}

std::tuple<int,int, ObjectId> Picker::at(vec2 uv) {
    const int x = static_cast<int>(uv.x);
    const int y = static_cast<int>(uv.y);
    const int w = static_cast<int>(rect[2]);
    const int h = static_cast<int>(rect[3]);
    // Vérification des bornes pour éviter tout crash hors image
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return {-1, -1, ObjectId()};
    }

    // Inversion de l'axe Y : l'origine OpenGL est en bas à gauche
    int flipped_y = h - 1 - y;

    int off = (flipped_y * w + x) * 4;
    int layer_id = decode(layer_ids, off);
    int vertex_id = decode(vertex_ids, off);
    auto layer_opt = God::layers.find_by_id(layer_id);
    
    


    // Retrieve primitive id from vertex id
    int primitive_id = -1;
    if (layer_opt.has_value()) {
        auto &layer = layer_opt.value().get();
        return {layer_id, layer.primitive_id(vertex_id), layer.mesh()};
    }

    return {-1, -1, ObjectId()};
}