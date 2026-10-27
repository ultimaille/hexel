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


TrianglesLayer::TrianglesLayer() { point_renderer.layer_id = id; tri_renderer.layer_id = id; }

void TrianglesLayer::generate_gui(std::string name){
    if (rt == RenderType::POINT_SPHERE)
        point_renderer.generate_gui(name);
    else 
        tri_renderer.generate_gui(name);
    Layer::generate_gui(name);
}

// Note: same as HexahedraLayer
void TrianglesLayer::init(ObjectId obj){
    mesh = obj;
    tri_renderer.init();
    point_renderer.init();
    bind_attribute({});
}

void TrianglesLayer::reset() {
    um_assert(mesh.ref() != std::nullopt);
    Triangles& tri = mesh;
    SurfaceAttributes& attr = mesh;

    God::shaders.create_if_needed(std::string(SHADERS_DIR), "triangle");
    God::shaders.create_if_needed(std::string(SHADERS_DIR), "point_as_sphere");
    FacetAttribute<bool> visible_tri("visible", attr, tri, true);

    if (attr_active()) {
        show_object_id(attr_id);
        if (rt == RenderType::POINT_SPHERE) {
            point_renderer.color_map_prop = 1;
        } else {
            tri_renderer.color_map_prop = 1;
        }
    }

    update();
}

void TrianglesLayer::update(){
    if (!attr_active() || rt == PLAIN) update_data_for_plain();
    else if (attr_type() == "bool") update_typed<bool>();
    else if (attr_type() == "char") update_typed<char>();
    else if (attr_type() == "int") update_typed<int>();
    else if (attr_type() == "float") update_typed<float>();
    else if (attr_type() == "double") update_typed<double>();
}

template<class T>
void TrianglesLayer::update_typed(){
    switch (rt) {
        case POINT_SPHERE: {
            // update_data_for_scalar_point_attribute_as_sphere<T>();
            break;
        }
        case FACET: {
            // update_data_for_scalar_facet_attribute<T>();
            break;
        }
        case CORNER: {
            // update_data_for_scalar_corner_attribute<T>();
            break;
        }
        case CORNER_GLYPH: {
            // update_data_for_scalar_corner_glyph_attribute<T>();
            break;
        }
        case POINT: {
            // update_data_for_scalar_point_attribute<T>();
            break;
        }
    }
}

void TrianglesLayer::update_data_for_plain() {
    Triangles &tri = mesh;
    FacetAttribute<bool> visible_tri(tri, true);
    FacetAttribute<int> primitive_id(tri, 0);
    CornerAttribute<float> value_tri(tri, 0);
    prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);
    tri_renderer.update(tri, visible_tri, value_tri, primitive_id);
}

void TrianglesLayer::prepare_data_without_attribute(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id) {
    for (auto &f : tri.iter_facets())
        primitive_id[f] = f;
}

// Note: same as HexahedraLayer
void TrianglesLayer::render(){
    if (!visible) return;
    if (rt==RenderType::POINT_SPHERE)
        point_renderer.render();
    else 
        tri_renderer.render();
}

// Note: same as HexahedraLayer
void TrianglesLayer::destroy() {
    tri_renderer.destroy();
    point_renderer.destroy();
}

// Note: almost the same as HexahedraLayer
void TrianglesLayer::bind_attribute(ObjectId p_attr_id){
    attr_id = p_attr_id;

    SurfaceAttributes& attr = mesh;

    if (attr_active()) {
        if (attr_primitive() == "facets")
            rt = RenderType::FACET;
        else if (attr_primitive() == "corners")
            if (rt!= RenderType::CORNER_GLYPH) rt = RenderType::CORNER;
        else if (attr_primitive() == "points")
            rt = RenderType::POINT;
    } else {
        rt = PLAIN;
    }

    reset();
};