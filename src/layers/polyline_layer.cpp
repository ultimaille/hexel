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
        std::string attrib_text = "ATTRIB: " + attr_id.collection() + "." + attr_id.name();// + " (" + attr_type() + ")";
        ImGui::TextUnformatted(attrib_text.c_str());
    }

    if (attr_active() && attr_id.kind() == AttributeBase::POINTS){
        bool tmp = (rt == RenderType::POINT_SPHERE);
        ImGui::Checkbox(label("Show vertices ", ""), &tmp);
        if ((rt == RenderType::POINT_SPHERE) != tmp) {
            if (tmp) rt = RenderType::POINT_SPHERE;
            else  rt = RenderType::POINT;
            reset();
        }
    }

    if (attr_active() && attr_id.kind() == AttributeBase::EDGES){
        bool tmp = (rt == RenderType::EDGE_REPEAT);
        ImGui::Checkbox(label("Show as repeat ", ""), &tmp);
        if ((rt == RenderType::EDGE_REPEAT) != tmp) {
            if (tmp) {
                rt = RenderType::EDGE_REPEAT;
                edge_renderer.texture_repeat = 1;
                edge_renderer.texture_range[0] = 0;
                edge_renderer.texture_range[1] = 1;
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

void PolyLineLayer::set_default_values() {
    auto& r = edge_renderer;
    r.color[0] = .5; r.color[1] = .5; r.color[2] = .7;
}


template<class T>
void PolyLineLayer::update_data_for_scalar_edge_attribute() {
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;
    EdgeAttribute<T> value_pl(attr_id.name(), attr, pl);
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
    EdgeAttribute<T> value_pl(attr_id.name(), attr, pl);
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
    PointAttribute<T> value(attr_id.name(), attr, pl);
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
    PointAttribute<T> value(attr_id.name(), attr, pl);
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
bool PolyLineLayer::update_typed(){
            auto ptr = dynamic_cast<const AttributeContainer<T>*>((ContainerBase *)attr_id);
            if (ptr == nullptr)
                return false;

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
    return true;
}

void PolyLineLayer::update(){

        if (!attr_active() || rt == PLAIN) {
            update_data_for_plain();
        } else {
            bool success =
                update_typed<bool>()  ||
                update_typed<char>()  ||
                update_typed<int>()   ||
                update_typed<float>() ||
                update_typed<double>();
            if (!success)
                Log::add("Display attribute of type that isn't currently supported by PolyLineLayer");
        }
}


void PolyLineLayer::reset() {
    PolyLine& pl = mesh;
    PolyLineAttributes& attr = mesh;

    God::shaders.create_if_needed(std::string(SHADERS_DIR), "segment_as_tube");
    God::shaders.create_if_needed(std::string(SHADERS_DIR), "point_as_sphere");
    EdgeAttribute<bool> visible_pl("visible", attr, pl, true);

    if (attr_active()) {
        if (rt == RenderType::POINT_SPHERE) {
            point_renderer.color_map_prop = 1;
        }
        else {
            edge_renderer.color_map_prop = 1;
        }
    }

    update();
}

void PolyLineLayer::init(MeshPath obj){
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

void PolyLineLayer::bind_attribute(AttributePath p_attr_id){
    attr_id = p_attr_id;
    PolyLineAttributes& attr = mesh;

    if (attr_active()) {
        if (attr_id.kind() == AttributeBase::EDGES)
            rt = RenderType::EDGE;
        else if (attr_id.kind() == AttributeBase::POINTS)
            rt = RenderType::POINT;
    }
    else {
        rt = PLAIN;
    }

    reset();
};

