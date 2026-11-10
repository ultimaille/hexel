#include "triangle_layer.h"
#include "core/layers.h"
#include "core/camera.h"
#include "core/xcf.h"
#include "core/render_target.h"
#include "core/window.h"
#include "core/panels.h"
#include "core/shaders.h"
#include "core/mode.h"

#include <misc/purgatory.h>
#include <layers/layer_ui.h>
#include <format>



TrianglesLayer::TrianglesLayer() { point_renderer.layer_id = id; tri_renderer.layer_id = id; }


int TrianglesLayer::primitive_type(){
    switch (rt){
    case RenderType::POINT_SPHERE: return 0;
    case RenderType::CORNER_GLYPH:
    case RenderType::HALFEDGE_GLYPH: 
        return 2;
    };
    return 1;
}


void TrianglesLayer::generate_gui(std::string name) {

    ImGui::PushID(name.c_str());

    LayerUI layer_ui(*this);
    layer_ui.show();

    ImGui::PopID();
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
    else {
        Log::add("Display attribute of type " + attr_type() + " isn't currently supported by TrianglesLayer");
    }
}

template<class T>
void TrianglesLayer::update_typed(){
    switch (rt) {
        case POINT_SPHERE: {
            update_data_for_scalar_point_attribute_as_sphere<T>();
            break;
        }
        case FACET: {
            update_data_for_scalar_facet_attribute<T>();
            break;
        }
        case CORNER: {
            update_data_for_scalar_corner_attribute<T>();
            break;
        }
        case CORNER_GLYPH: {
            update_data_for_scalar_corner_glyph_attribute<T>();
            break;
        }
        case HALFEDGE_GLYPH: {
            Log::add("To be implemented");
            // update_data_for_scalar_halfedge_glyph_attribute<T>();
            break;
        }
        case POINT: {
            update_data_for_scalar_point_attribute<T>();
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

template<class T>
void TrianglesLayer::update_data_for_scalar_facet_attribute() {
    Triangles &tri = mesh;
    SurfaceAttributes& attr = mesh;

    FacetAttribute<bool> visible_tri("visible", attr, tri, true);
    FacetAttribute<int> primitive_id(tri, 0);
    CornerAttribute<float> value_tri(tri, 0);
    FacetAttribute<T> value(attr_name(), attr, tri);
    prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);
    for (auto h : tri.iter_halfedges()) value_tri[h] =  static_cast<float>(value[h / 3]);
    tri_renderer.update(tri, visible_tri, value_tri, primitive_id);
}

template<class T>
void TrianglesLayer::update_data_for_scalar_corner_attribute() {
    Triangles &tri = mesh;
    SurfaceAttributes& attr = mesh;

    FacetAttribute<bool> visible_tri("visible", attr, tri, true);
    FacetAttribute<int> primitive_id(tri, 0);
    CornerAttribute<float> value_tri(tri, 0);
    CornerAttribute<T> value(attr_name(), attr, tri);

    prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);

    for (auto &h : tri.iter_halfedges()) value_tri[h] = static_cast<float>(value[h]);

    tri_renderer.update(tri, visible_tri, value_tri, primitive_id);
}

template<class T>
void TrianglesLayer::update_data_for_scalar_corner_glyph_attribute() {
    Triangles &tri = mesh;
    SurfaceAttributes& attr = mesh;

    FacetAttribute<bool> visible_tri("visible", attr, tri, true);
    CornerAttribute<T> value(attr_name(), attr, tri);

    Triangles tri_glyph;
    tri_glyph.points.create_points(tri.ncorners() * 3);
    tri_glyph.create_facets(tri.ncorners());

    FacetAttribute<bool> visible_glyph(tri_glyph);
    CornerAttribute<float> value_glyph(tri_glyph);
    FacetAttribute<int> primitive_id(tri_glyph);

    for (auto &f : tri.iter_facets()) {
        Triangle3 t = f;
        vec3 bary = t.bary_verts();
        for (int lv = 0; lv < 3; ++lv) {
            vec3 from = f.vertex(lv);
            vec3 to = f.vertex((lv + 1) % 3);
            vec3 opp = f.vertex((lv + 2) % 3);
            auto h = f.halfedge(lv);

            vec3 a = from * (1. - shrink) + bary * shrink;
            vec3 b = to * (1. - shrink) + bary * shrink;
            vec3 c = opp * (1. - shrink) + bary * shrink;

            vec3 d0 = (c - a);
            vec3 d1 = (c - b);

            // opp
            float size = corner_size * .5;

            tri_glyph.points[h * 3] = a * size + c * (1. - size);
            tri_glyph.points[h * 3 + 1] = b * size + c * (1. - size);
            tri_glyph.points[h * 3 + 2] = c;

            for (int i = 0; i < 3; ++i) {
                tri_glyph.vert(h, i) = h * 3 + i;

                value_glyph[h * 3 + i] = static_cast<float>(value[h]);
                visible_glyph[h] = visible_tri[f];
                primitive_id[h] = h;
            }

        }
    }

    tri_renderer.update(tri_glyph, visible_glyph, value_glyph, primitive_id);
}

template<class T> 
void TrianglesLayer::update_data_for_scalar_point_attribute() {
    Triangles &tri = mesh;
    SurfaceAttributes& attr = mesh;

    FacetAttribute<bool> visible_tri("visible", attr, tri, true);
    FacetAttribute<int> primitive_id(tri, 0);
    CornerAttribute<float> value_tri(tri, 0);
    PointAttribute<T> value(attr_name(), attr, tri);

    prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);

    // Note: need the mesh to be connected...
    // for (auto &h : tri.iter_halfedges()) value_tri[h] = value[h.from()];
    // Note: below no need the mesh to be connected
    for (auto &f : tri.iter_facets()) {
        for (int lv = 0; lv < 3; ++lv) {
            auto h = f.halfedge(lv);
            auto v = f.vertex(lv);
            value_tri[h] = static_cast<float>(value[v]);
        }
    }

    tri_renderer.update(tri, visible_tri, value_tri, primitive_id);
}

template<class T> void TrianglesLayer::update_data_for_scalar_point_attribute_as_sphere() {
    Triangles &tri = mesh;
    SurfaceAttributes& attr = mesh;

    FacetAttribute<bool> visible_tri("visible", attr, tri, true);
    PointAttribute<T> value(attr_name(), attr, tri);

    PointAttribute<bool> visible_pts(tri,false);
    PointAttribute<float> value_float(tri);
    PointAttribute<int> primitive_id(tri.points);

    for (auto &f : tri.iter_facets()) {
        for (int lv = 0; lv < 3; ++lv) {
            auto v = f.vertex(lv);
            visible_pts[v] = visible_tri[f];
            value_float[v] = static_cast<float>(value[v]);
            primitive_id[v] = v;
        }
    }

    point_renderer.update(tri.points, visible_pts, value_float, primitive_id);
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
        else if (attr_primitive() == "corners") {
            if (rt!= RenderType::CORNER_GLYPH && rt != RenderType::HALFEDGE_GLYPH) 
                rt = RenderType::CORNER;
        } else if (attr_primitive() == "points")
            rt = RenderType::POINT;
    } else {
        rt = PLAIN;
    }

    reset();
};