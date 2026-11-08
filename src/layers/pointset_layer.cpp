#include "pointset_layer.h"
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

PointSetLayer::PointSetLayer() { point_renderer.layer_id = id; }

void PointSetLayer::generate_gui(std::string name){
    point_renderer.generate_gui(name);

    
    if (ImGui::Checkbox(label("mask a value##autorange", name), &mask))update();
    if (mask) if (ImGui::InputInt(label("mask value", name), &skip_value)){ plop(skip_value); update(); }

    Layer::generate_gui(name);
}

void PointSetLayer::set_default_values() {
    auto& r = point_renderer;
    r.color[0] = .5; r.color[1] = 1.; r.color[2] = .5;
    r.radius_in_pixel = 4;
}


int PointSetLayer::primitive_type(){ return 0; }

void PointSetLayer::init(ObjectId obj){
    mesh = obj;
    point_renderer.init();
    reset();
}

void PointSetLayer::reset() {
    um_assert(mesh.ref() != std::nullopt);
    PointSet& ps = mesh;
    PointSetAttributes& attr = mesh;

    God::shaders.create_if_needed(std::string(SHADERS_DIR), "point_as_sphere");
    PointAttribute<bool> visible_pts("visible", attr, ps, true);

    if (selected_attr.has_value()) {
        point_renderer.color_map_prop = 1;
    }

    update();
}

void PointSetLayer::update(){
    if (!selected_attr.has_value() || rt == PLAIN) update_data_for_plain();
    else if (selected_attr.value().type == "bool") update_typed<bool>();
    else if (selected_attr.value().type == "char") update_typed<char>();
    else if (selected_attr.value().type == "int") update_typed<int>();
    else if (selected_attr.value().type == "float") update_typed<float>();
    else if (selected_attr.value().type == "double") update_typed<double>();
    else {
        Log::add("Display attribute of type " + selected_attr.value().type + " isn't currently supported by PointSetLayer");
    }
}

template<class T>
void PointSetLayer::update_typed(){
    switch (rt) {
        case ATTRIBUTE_SPHERE: {
            update_data_for_scalar_point_attribute_as_sphere<T>();
            break;
        }
    }
}

void PointSetLayer::update_data_for_plain() {
    PointSet &ps = mesh;
    PointSetAttributes &attr = mesh;
    PointAttribute<bool> visible_pts("visible", attr, ps, true);
    PointAttribute<int> primitive_id(ps, 0);
    PointAttribute<float> value_pts(ps, 0);

    for (int v = 0; v < ps.size(); ++v) primitive_id[v] = v;
    if (!mask)
        point_renderer.update(ps, visible_pts, value_pts, primitive_id);
    else{
        PointAttribute<bool> masked_visible_pts(ps);
        for (int v = 0; v < ps.size(); ++v)  masked_visible_pts[v] = visible_pts[v] && value_pts[v] != skip_value;
        point_renderer.update(ps, masked_visible_pts, value_pts, primitive_id);
    }
}

template<class T> 
void PointSetLayer::update_data_for_scalar_point_attribute_as_sphere() {
    PointSet &ps = mesh;
    PointSetAttributes &attr = mesh;
    PointAttribute<bool> visible_pts("visible", attr, ps, true);
    PointAttribute<int> primitive_id(ps, 0);
    PointAttribute<T> value_pts(selected_attr.value().name, attr, ps);
    PointAttribute<float> value_float(ps, 0);

    for (int v = 0; v < ps.size(); ++v) {
        value_float[v] = static_cast<float>(value_pts[v]);
        primitive_id[v] = v;
    }
     if (!mask)
         point_renderer.update(ps, visible_pts, value_float, primitive_id);
     else{
         PointAttribute<bool> masked_visible_pts(ps);
         for (int v = 0; v < ps.size(); ++v)  masked_visible_pts[v] = visible_pts[v] && value_float[v] != skip_value;
         point_renderer.update(ps, masked_visible_pts, value_float, primitive_id);
     }
}

// Note: partially the same as HexahedraLayer
void PointSetLayer::bind_attribute(AttributeInfo attr_info){
    selected_attr = attr_info;

    if (!selected_attr.has_value()) {
        rt = RenderType::PLAIN;
    } else if (selected_attr.value().primitive == AttributeBase::TYPE::POINTS) {
        rt = RenderType::ATTRIBUTE_SPHERE;
    }

    reset();
};

void PointSetLayer::render(){
    if (visible) point_renderer.render();
}

void PointSetLayer::destroy() {
    point_renderer.destroy();
}
