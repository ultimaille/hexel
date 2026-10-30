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
    Layer::generate_gui(name);
}

int PointSetLayer::primitive_type(){ return 0; }

void PointSetLayer::init(ObjectId obj){
    mesh = obj;
    point_renderer.init();
    bind_attribute({});
}

void PointSetLayer::reset() {
    um_assert(mesh.ref() != std::nullopt);
    PointSet& ps = mesh;
    PointSetAttributes& attr = mesh;

    God::shaders.create_if_needed(std::string(SHADERS_DIR), "point_as_sphere");
    PointAttribute<bool> visible_pts("visible", attr, ps, true);

    if (attr_active()) {
        show_object_id(attr_id);
        point_renderer.color_map_prop = 1;
    }

    update();
}

void PointSetLayer::update(){
    if (!attr_active() || rt == PLAIN) update_data_for_plain();
    else if (attr_type() == "bool") update_typed<bool>();
    else if (attr_type() == "char") update_typed<char>();
    else if (attr_type() == "int") update_typed<int>();
    else if (attr_type() == "float") update_typed<float>();
    else if (attr_type() == "double") update_typed<double>();
}

template<class T>
void PointSetLayer::update_typed(){
    switch (rt) {
        case POINT_SPHERE: {
            update_data_for_scalar_point_attribute_as_sphere<T>();
            break;
        }
    }
}

void PointSetLayer::update_data_for_plain() {
    PointSet &ps = mesh;
    PointAttribute<bool> visible_pts(ps, true);
    PointAttribute<int> primitive_id(ps, 0);
    PointAttribute<float> value_pts(ps, 0);
    point_renderer.update(ps, visible_pts, value_pts, primitive_id);
}

template<class T> 
void PointSetLayer::update_data_for_scalar_point_attribute_as_sphere() {
    PointSet &ps = mesh;
    PointSetAttributes &attr = mesh;
    PointAttribute<bool> visible_pts("visible", attr, ps, true);
    PointAttribute<int> primitive_id(ps, 0);
    PointAttribute<T> value_pts(attr_name(), attr, ps);
    PointAttribute<float> value_float(attr_name(), attr, ps);

    for (int v = 0; v < ps.size(); ++v) {
        value_float[v] = static_cast<float>(value_pts[v]);
        primitive_id[v] = v;
    }

    point_renderer.update(ps, visible_pts, value_float, primitive_id);
}

template<class T> 
void PointSetLayer::update_data_for_scalar_point_attribute_as_sized_sphere() {
    PointSet &ps = mesh;
    PointSetAttributes &attr = mesh;
    PointAttribute<bool> visible_pts("visible", attr, ps, true);
    PointAttribute<int> primitive_id(ps, 0);
    PointAttribute<T> value_pts(attr_name(), attr, ps);
    // PointAttribute<bool> value_bool(attr_name(), attr, ps);

    for (int v = 0; v < ps.size(); ++v) {
        primitive_id[v] = v;
    }




    point_renderer.update(ps, visible_pts, value_pts, primitive_id);
}

// Note: partially the same as HexahedraLayer
void PointSetLayer::bind_attribute(ObjectId p_attr_id){
    attr_id = p_attr_id;

    PointSetAttributes& attr = mesh;

    if (attr_active()) {
        if (attr_primitive() == "points")
            rt = RenderType::POINT_SPHERE;
    } else {
        rt = PLAIN;
    }

    reset();
};

void PointSetLayer::render(){
    if (visible) point_renderer.render();
}

void PointSetLayer::destroy() {
    point_renderer.destroy();
}
