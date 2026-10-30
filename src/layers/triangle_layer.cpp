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

void TrianglesLayer::generate_gui(std::string name) {

    if (attr_active() && attr_primitive() == "points"){
        bool tmp = (rt==RenderType::POINT_SPHERE);
            ImGui::Checkbox(label("Show vertices ", ""), &tmp);
        if ((rt == RenderType::POINT_SPHERE) != tmp) {
            if (tmp) rt = RenderType::POINT_SPHERE;
            else  rt = RenderType::POINT;
            reset();
        }
    }

    if (attr_active() && attr_primitive() == "corners") {
        bool tmp = (rt == RenderType::CORNER_GLYPH);
        
        ImGui::Checkbox(label("render_corners_as_glyphs ", ""), &tmp);
        
        if ((rt == RenderType::CORNER_GLYPH) != tmp){
            if (tmp) rt = RenderType::CORNER_GLYPH;
            else rt = RenderType::CORNER;
            reset();
        }

        bool is_halfedge_glyphs = (rt == RenderType::HALFEDGE_GLYPH);

        ImGui::Checkbox(label("render_corners_as_halfedge_glyphs ", ""), &is_halfedge_glyphs);

        if ((rt == RenderType::HALFEDGE_GLYPH) != is_halfedge_glyphs) {
            rt = is_halfedge_glyphs ? RenderType::HALFEDGE_GLYPH : RenderType::CORNER;
            reset();
        }

    }

    if (rt == RenderType::CORNER_GLYPH) {
        float tmp_shrink = shrink;
        float tmp_corner_size = corner_size;
        ImGui::SliderFloat(label("shrink", name), &shrink, 0.0f, .9f, "%.2f", 0);
        ImGui::SliderFloat(label("corner_size", name), &corner_size, 0.1f, 1.f, "%.2f", 0);
        if (tmp_shrink != shrink || tmp_corner_size != corner_size) reset();
    }

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
void TrianglesLayer::update_data_for_scalar_halfedge_glyph_attribute() {
    Triangles &tri = mesh;
    SurfaceAttributes& attr = mesh;

    FacetAttribute<bool> visible_tri("visible", attr, tri, true);
    CornerAttribute<T> value(attr_name(), attr, tri);

    // Note: need mesh to be connected
    Triangles tri_glyph;
    tri_glyph.points.create_points(tri.ncorners() * 6); // 4 point for deinfe a rect by halfedge
    tri_glyph.create_facets(tri.ncorners() * 2);
    
    FacetAttribute<bool> visible_glyph(tri_glyph);
    CornerAttribute<float> value_glyph(tri_glyph);
    FacetAttribute<int> primitive_id(tri_glyph);

    // for (auto &f : tri.iter_facets()) {
    //     for (int lv = 0; lv < 3; ++lv) {
    //         auto from = f.vertex(lv);
    //         auto to = f.vertex((lv + 1) % 3);
    //         auto h = f.halfedge(lv);
    //     }
    // }

    // for (auto &h : tri.iter_halfedges()) {
    //     auto from = h.from();
    //     auto to = h.to();
    // }

    for (auto &h : tri.iter_halfedges()) {
        auto v0 = h.from();
        auto v1 = h.to();
        vec3 p0 = v0;
        vec3 p1 = v1;
        vec3 d = (p1 - p0).normalized();
        vec3 perp{-d.y, d.x};

        p0 += d * 0.005;
        p1 -= d * 0.005;

        int f0 = h * 2;
        int f1 = h * 2 + 1;


        // Create rect
        tri_glyph.points[h * 6] = p0 + perp * 0.01;
        tri_glyph.points[h * 6 + 1] = p0 - perp * 0.01;
        tri_glyph.points[h * 6 + 2] = p1 - perp * 0.01;
        tri_glyph.vert(h * 2, 0) = h * 6;
        tri_glyph.vert(h * 2, 1) = h * 6 + 1;
        tri_glyph.vert(h * 2, 2) = h * 6 + 2;

        tri_glyph.points[h * 6 + 3] = p1 - perp * 0.01;
        tri_glyph.points[h * 6 + 4] = p1 + perp * 0.01;
        tri_glyph.points[h * 6 + 5] = p0 + perp * 0.01;
        tri_glyph.vert(h * 2 + 1, 0) = h * 6 + 3;
        tri_glyph.vert(h * 2 + 1, 1) = h * 6 + 4;
        tri_glyph.vert(h * 2 + 1, 2) = h * 6 + 5;

        visible_glyph[f0] = visible_tri[h.facet()];
        visible_glyph[f1] = visible_tri[h.facet()];
        float val = static_cast<float>(value[h]);
        for (int i = 0; i < 6; ++i)
            value_glyph[h * 6 + i] = val;
        
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