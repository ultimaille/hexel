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

PolyLineLayer::PolyLineLayer()  { point_renderer.layer_id = id; edge_renderer.layer_id = id; }

int PolyLineLayer::primitive_type(){
    switch (rt){
    case RenderType::PLAIN: 
    case RenderType::EDGE: 
    case RenderType::EDGE_REPEAT: 
    case RenderType::POINT: 
        return 1;
    case RenderType::POINT_SPHERE: 
        return 0;
    };
    Log::abort("Primitive type does not exist");
    return -1;
}


void PolyLineLayer::generate_gui(std::string name){
    if (attr_active()){
        std::string attrib_text = "ATTRIB: " + attr_primitive() + "." + attr_name() + " (" + attr_type() + ")";
        ImGui::Text(attrib_text.c_str());
    }

    if (attr_active() && attr_primitive() == "points"){
        bool tmp = (rt == RenderType::POINT_SPHERE);
        ImGui::Checkbox(label("Show vertices ", ""), &tmp);
        if ((rt == RenderType::POINT_SPHERE) != tmp) {
            if (tmp) rt = RenderType::POINT_SPHERE;
            else  rt = RenderType::POINT;
            reset();
        }
    }

    if (attr_active() && attr_primitive() == "edges"){
        bool tmp = (rt == RenderType::EDGE_REPEAT);
        ImGui::Checkbox(label("Show as repeat ", ""), &tmp);
        if ((rt == RenderType::EDGE_REPEAT) != tmp) {
            if (tmp) {
                rt = RenderType::EDGE_REPEAT;
                edge_renderer.texture_repeat = 1;
                edge_renderer.data_range[0] = 0;
                edge_renderer.data_range[1] = 1;
            }
            else  rt = RenderType::EDGE;
            reset();
        }
    }

    if (rt == RenderType::POINT_SPHERE)
        point_renderer.generate_gui(name);
    else
        edge_renderer.generate_gui(name);
    Layer::generate_gui(name);
}


template<class T>
void PolyLineLayer::update_data_for_scalar_edge_attribute() {
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;
    EdgeAttribute<T> value_pl(attr_name(), attr, pl);
    EdgeAttribute<bool> visible("visible", attr, pl, true);
    EdgeAttribute<int> primitive_id(pl);
    for (auto e : pl.iter_edges()) primitive_id[e] = e;

    PolyLine pl_render ;
    pl_render.points.create_points(2*pl.nedges());
    pl_render.create_edges(pl.nedges());

    PointAttribute<float> value(pl_render, 0);
    for (auto e : pl.iter_edges()) {
        FOR(lv,2) pl_render.vert(e, lv) = 2 * e + lv;
        pl_render.points[2 * e + 0]= e.from().pos();
        pl_render.points[2 * e + 1]= e.to().pos();
        value[2 * e + 0] = value_pl[e];
        value[2 * e + 1] = value_pl[e];
    }
    edge_renderer.update(pl_render, visible, value, primitive_id);
}
template<class T>
void PolyLineLayer::update_data_for_scalar_edge_attribute_as_repeat() {
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;
    EdgeAttribute<T> value_pl(attr_name(), attr, pl);
    EdgeAttribute<bool> visible("visible", attr, pl, true);
    EdgeAttribute<int> primitive_id(pl);
    for (auto e : pl.iter_edges()) primitive_id[e] = e;

    PolyLine pl_render;
    pl_render.points.create_points(2 * pl.nedges());
    pl_render.create_edges(pl.nedges());

    PointAttribute<float> value(pl_render, 0);
    for (auto e : pl.iter_edges()) {
        FOR(lv, 2) pl_render.vert(e, lv) = 2 * e + lv;
        pl_render.points[2 * e + 0] = e.from().pos();
        pl_render.points[2 * e + 1] = e.to().pos();
        value[2 * e + 0] = 0;
        value[2 * e + 1] = value_pl[e];
    }
    edge_renderer.update(pl_render, visible, value, primitive_id);
}

template<class T>
void PolyLineLayer::update_data_for_scalar_point_attribute() {
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;

    EdgeAttribute<bool> visible("visible", attr, pl, true);
    EdgeAttribute<int> primitive_id(pl);
    PointAttribute<T> value(attr_name(), attr, pl);
    PointAttribute<float> value_float(pl);
    FOR(v, pl.nverts()) value_float[v] = value[v];
    for (auto e : pl.iter_edges())
        primitive_id[e] = e;
    edge_renderer.update(pl, visible, value_float, primitive_id);
}

template<class T> void PolyLineLayer::update_data_for_scalar_point_attribute_as_sphere(){
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;
    EdgeAttribute<bool> visible_pl("visible", attr, pl, true);
    PointAttribute<T> value(attr_name(), attr, pl);
    PointAttribute<float> value_float(pl);
    FOR(v, pl.nverts()) value_float[v] = value[v];
    PointAttribute<bool> visible_pts(pl, false);
    for (auto e : pl.iter_edges()) if (visible_pl[e]) {
        visible_pts[e.from()] = true;
        visible_pts[e.to()] = true;
    }
    PointAttribute<int> primitive_id(pl.points);
    FOR(v, pl.points.size())primitive_id[v] = v;

    point_renderer.update(pl.points, visible_pts, value_float, primitive_id);
}

void PolyLineLayer::update_data_for_plain() {
        PolyLine& pl = mesh;
        PolyLineAttributes& attr = mesh;
        PointAttribute<float> value(pl, 0);
        for (auto v : pl.iter_vertices())
            value[v] = ((vec3)v)[0];
    
        EdgeAttribute<bool> visible("visible", attr, pl, true);
        EdgeAttribute<int> primitive_id(pl);
        for (auto e : pl.iter_edges())
            primitive_id[e] = e;
        edge_renderer.update(pl, visible, value, primitive_id);
}



template<class T>
void PolyLineLayer::update_typed(){
    switch (rt) {
    case EDGE: {
        update_data_for_scalar_edge_attribute<T>();
        break;
    }
    case EDGE_REPEAT: {
        update_data_for_scalar_edge_attribute_as_repeat<T>();
        break;
    }
    case POINT: {
        update_data_for_scalar_point_attribute<T>();
        break;
    }
    case POINT_SPHERE: {
        update_data_for_scalar_point_attribute_as_sphere<T>();
        break;
    }
    }
}

void PolyLineLayer::update(){
    if (!attr_active() || rt == PLAIN) update_data_for_plain();
    else if (attr_type() == "bool") update_typed<bool>();
    else if (attr_type() == "char") update_typed<char>();
    else if (attr_type() == "int") update_typed<int>();
    else if (attr_type() == "float") update_typed<float>();
    else if (attr_type() == "double") update_typed<double>();
}


void PolyLineLayer::reset() {
    um_assert(mesh.ref() != std::nullopt);
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;

    God::shaders.create_if_needed(std::string(SHADERS_DIR), "segment_as_tube");
    God::shaders.create_if_needed(std::string(SHADERS_DIR), "point_as_sphere");
    EdgeAttribute<bool> visible_pl("visible", attr, pl, true);
    show_object_id(attr_id);

    if (attr_active()) {
        show_object_id(attr_id);
        if (rt == RenderType::POINT_SPHERE) {
            point_renderer.color_map_prop = 1;
        }
        else {
            edge_renderer.color_map_prop = 1;
        }
    }

    update();
}

void PolyLineLayer::init(ObjectId obj){
    mesh = obj;
    edge_renderer.init();
    point_renderer.init();
    bind_attribute({});
}

void PolyLineLayer::render(){
    if (!visible) return;
    if (rt == RenderType::POINT_SPHERE)
        point_renderer.render();
    else
        edge_renderer.render();
}


void PolyLineLayer::destroy() {
    edge_renderer.destroy();
    point_renderer.destroy();
}

void PolyLineLayer::bind_attribute(ObjectId p_attr_id){
    attr_id = p_attr_id;
    PolyLineAttributes& attr = mesh;

    if (attr_active()) {
        if (attr_primitive() == "edges")
            rt = RenderType::EDGE;
        else if (attr_primitive() == "points")
            rt = RenderType::POINT;
    }
    else {
        rt = PLAIN;
    }

    reset();
};

