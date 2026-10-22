#include "triangle_layer.h"
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


TrianglesLayer::TrianglesLayer() { primitive_renderer.layer_id = id; }

void TrianglesLayer::generate_gui(std::string name){
    primitive_renderer.generate_gui(name);
    Layer::generate_gui(name);
}

void TrianglesLayer::reset() {
    init(mesh);
}


void TrianglesLayer::init(ObjectId obj){
    mesh = obj;
    um_assert(obj.ref() != std::nullopt);
    Triangles& tri = obj;
    SurfaceAttributes& attr = obj;

    God::shaders.create_if_needed(std::string(SHADERS_DIR), "triangle");

    CornerAttribute<float> value(tri);
    for (auto h : tri.iter_halfedges())  {
        value[h] = ((vec3)h.from())[0];
    }

    FacetAttribute<bool> visible(true);
    visible.bind("visible", attr, tri);
    FacetAttribute<int> primitive_id(tri);
    for (auto f : tri.iter_facets()) primitive_id[f] = f;
    primitive_renderer.init();
    primitive_renderer.update(tri, visible, value,primitive_id);
}

void TrianglesLayer::render(){
    if (visible)
        primitive_renderer.render();
}

int TrianglesLayer::primitive_id(int vertex_id) {
    return vertex_id; // triangle id from vertex id
}

void TrianglesLayer::destroy() {
    primitive_renderer.destroy();
}