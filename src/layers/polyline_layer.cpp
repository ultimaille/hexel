#include "polyline_layer.h"
#include "core/layers.h"
#include "core/camera.h"
#include "core/picker.h"
#include "core/xcf.h"
#include "core/render_target.h"
#include "core/window.h"
#include "core/panels.h"
#include "core/shaders.h"
#include "core/mode.h"

#include <misc/purgatory.h>
#include <format>

PolyLineLayer::PolyLineLayer()  { primitive_renderer.layer_id = id; }

void PolyLineLayer::generate_gui(std::string name) {
    primitive_renderer.generate_gui(name);
    Layer::generate_gui(name);
}


void PolyLineLayer::reset(){
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;
    PointAttribute<float> value(pl, 0);
    for (auto v : pl.iter_vertices())
        value[v] = ((vec3)v)[0];

    EdgeAttribute<bool> visible("visible", attr, pl, true);
    EdgeAttribute<int> primitive_id(pl);
    for (auto e : pl.iter_edges())
        primitive_id[e] = e;
    primitive_renderer.init();
    primitive_renderer.push(pl, visible, value, primitive_id);
    
}
void PolyLineLayer::init(ObjectId obj){
    mesh = obj;
    reset();
}

void PolyLineLayer::render(){
    if (visible)primitive_renderer.render();
}

int PolyLineLayer::primitive_id(int vertex_id) {
    return vertex_id; // edge id from vertex id
}

void PolyLineLayer::destroy() {
    primitive_renderer.destroy();
}
