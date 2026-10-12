#include <layers/hexahedra_layer.h>
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


    HexahedraLayer::HexahedraLayer()  { point_renderer.layer_id = id; tri_renderer.layer_id = id;}


    enum RenderType {
        PLAIN,
        CELL,
        FACET,
        CORNER, CORNER_GLYPH,
        POINT, POINT_SPHERE
    } rt;

    int HexahedraLayer::primitive_type(){
        switch (rt){
        case RenderType::PLAIN: return 2;
        case RenderType::CELL: return 2;
        case RenderType::FACET: return 2;
        case RenderType::CORNER: return 2;
        case RenderType::CORNER_GLYPH: return 3;
        case RenderType::POINT: return 2;
        case RenderType::POINT_SPHERE: return 0;
        };
    }


    void HexahedraLayer::generate_gui(std::string name){
        if (attr_active()){
            std::string attrib_text = "ATTRIB: " + attr_primitive() + "." + attr_name() + " (" + attr_type() + ")";
            ImGui::Text(attrib_text.c_str());
        }
        {
            float tmp = shrink;
            ImGui::SliderFloat(label("shrink", name), &shrink, 0.0f, .9f, "%.2f", 0);
            if (tmp != shrink) reset();
        }

        if (attr_active() && attr_primitive() == "points"){
            bool tmp = (rt==RenderType::POINT_SPHERE);
                ImGui::Checkbox(label("Show vertices ", ""), &tmp);
            if ((rt == RenderType::POINT_SPHERE) != tmp) {
                if (tmp) rt = RenderType::POINT_SPHERE;
                else  rt = RenderType::POINT;
                reset();
            }
        }

        if (attr_active() && attr_primitive() == "corners"){
            bool tmp = (rt == RenderType::CORNER_GLYPH);
            ImGui::Checkbox(label("render_corners_as_glyphs ", ""), &tmp);
            if ((rt == RenderType::CORNER_GLYPH) != tmp){
                if (tmp) rt = RenderType::CORNER_GLYPH;
                else rt = RenderType::CORNER;
                reset();
            }
        }

        if (rt == RenderType::POINT_SPHERE)
            point_renderer.generate_gui(name);
        else 
            tri_renderer.generate_gui(name);
        Layer::generate_gui(name);
    }

    void HexahedraLayer::prepare_data_without_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    ){
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;
        CellAttribute<bool> visible_hex("visible", attr, hex, true);
        tri.points.create_points(hex.ncorners());
        tri.create_facets(2 * hex.nfacets());
        for (auto c : hex.iter_corners()) tri.points[c] = c.vertex();
        for (auto c : hex.iter_cells()){
            vec3 G = Hexahedron(c).bary_verts();
            for (auto v : c.iter_corners())
                tri.points[v] = (1. - shrink) * tri.points[v] + shrink * G;
        }
        int map[2][3] = { {0,1,2},{2,3,0} };
        for (auto f : hex.iter_facets()) 
            FOR(t, 2)FOR(v, 3)tri.vert(2 * f + t,v) = f.corner(map[t][v]);
        for (auto f : hex.iter_facets()) FOR(i, 2) visible_tri[2 * f + i] = visible_hex[f.cell()];
        for (auto f : hex.iter_facets()) FOR(t, 2) primitive_id[2 * f + t] = f;

    }

    template<class T>
    void HexahedraLayer::prepare_data_for_scalar_cell_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    ){
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;
        CellAttribute<bool> visible_hex("visible", attr, hex, true);
        CellAttribute<T> value_hex(attr_name(), attr, hex);
        prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);
        for (auto h : tri.iter_halfedges()) value_tri[h] = value_hex[h / 36];
    }
    template<class T>
    void HexahedraLayer::prepare_data_for_scalar_facet_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    ){
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;
        CellAttribute<bool> visible_hex("visible", attr, hex, true);
        CellFacetAttribute<T> value_hex(attr_name(), attr, hex);
        prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);
        for (auto h : tri.iter_halfedges()) value_tri[h] =  value_hex[h / 6];
    }
    template<class T>
    void HexahedraLayer::prepare_data_for_scalar_corner_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    ){
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;
        CellAttribute<bool> visible_hex("visible", attr, hex, true);
        CellCornerAttribute<T> value_hex(attr_name(), attr, hex);
        prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);
        int map[2][3] = { {0,1,2},{2,3,0} };
        for (auto f : hex.iter_facets())
            FOR(t, 2)FOR(v, 3)value_tri[3 * (2 * f + t) + v] = value_hex[f.corner(map[t][v])];
    }
    template<class T>
    void HexahedraLayer::prepare_data_for_scalar_corner_glyph_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    ){
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;
        CellAttribute<bool> visible_hex("visible", attr, hex, true);

        CellCornerAttribute<T> value_hex(attr_name(), attr, hex);

        tri.points.create_points(3*hex.nhalfedges());
        tri.create_facets(hex.nhalfedges());
        for (auto h : hex.iter_halfedges()) {
            vec3 G = Hexahedron(h.cell()).bary_verts();
            vec3 A = h.from().pos();
            vec3 B = h.to().pos();
            vec3 C = h.prev().from().pos();
            double corner_size = .3;//[0..0.5]
            tri.points[3 * h] = (1. - shrink) * A + shrink * G;
            tri.points[3 * h+1] = (1. - shrink) * ((1.- corner_size)*A+ corner_size*B) + shrink * G;
            tri.points[3 * h+2] = (1. - shrink) * ((1. - corner_size) * A + corner_size * C) + shrink * G;
            visible_tri[h] = visible_hex[h.cell()];
            primitive_id[h] = h.from_corner();
            FOR(v, 3){
                tri.vert(h, v) = 3 * h + v;
                value_tri[3 * h + v] = value_hex[h.from_corner()];
            }
        }
    }

    template<class T> 
    void HexahedraLayer::prepare_data_for_scalar_point_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    ){
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;
        CellAttribute<bool> visible_hex("visible", attr, hex, true);

        Log::add("::prepare_data_for_scalar_point_attribute(");
        PointAttribute<T> value_hex(attr_name(), attr, hex);
        prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);
        int map[2][3] = { {0,1,2},{2,3,0} };
        for (auto f : hex.iter_facets())
            FOR(t, 2)FOR(v, 3)value_tri[3 * (2 * f + t)+v] = value_hex[f.vertex(map[t][v])];

    }

    template<class T> void HexahedraLayer::update_data_for_scalar_point_attribute_as_sphere(){
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;
        CellAttribute<bool> visible_hex("visible", attr, hex, true);
        PointAttribute<T> value(attr_name(), attr, hex);
        PointAttribute<float> value_float(hex);
        PointAttribute<bool> visible_pts(hex,false);
        FOR(v, hex.nverts()) value_float[v] = value[v];
        for (auto c : hex.iter_cells()) if (visible_hex[c]) FOR(lv,8)visible_pts[c.vertex(lv)]=true;
        
        PointAttribute<int> primitive_id(hex.points);
        FOR(v, hex.points.size())primitive_id[v] = v;

        point_renderer.update(hex.points, visible_pts, value_float, primitive_id);
    }


    
    template<class T>
    void HexahedraLayer::update_typed(){
        if (rt == RenderType::POINT_SPHERE){
            update_data_for_scalar_point_attribute_as_sphere<T>();
            return;
        }else{
            Triangles tri;
            FacetAttribute<bool> visible_tri(tri, true);
            FacetAttribute<int> primitive_id(tri, 0);
            CornerAttribute<float> value_tri(tri, 0);
            if (rt == RenderType::CELL)          prepare_data_for_scalar_cell_attribute<T>(tri, visible_tri, value_tri, primitive_id);
            if (rt == RenderType::FACET)         prepare_data_for_scalar_facet_attribute<T>(tri, visible_tri, value_tri, primitive_id);
            if (rt == RenderType::CORNER)        prepare_data_for_scalar_corner_attribute<T>(tri, visible_tri, value_tri, primitive_id);
            if (rt == RenderType::CORNER_GLYPH)  prepare_data_for_scalar_corner_glyph_attribute<T>(tri, visible_tri, value_tri, primitive_id);
            if (rt == RenderType::POINT)         prepare_data_for_scalar_point_attribute<T>(tri, visible_tri, value_tri, primitive_id);
            tri_renderer.update(tri, visible_tri, value_tri, primitive_id);
        }
    }

    void HexahedraLayer::update(){
        if (attr_type() == "bool") update_typed<bool>();
        if (attr_type() == "char") update_typed<char>();
        if (attr_type() == "int") update_typed<int>();
        if (attr_type() == "float") update_typed<float>();
        if (attr_type() == "double") update_typed<double>();
    }


    void HexahedraLayer::reset() {
        um_assert(mesh.ref() != std::nullopt);
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;

        God::shaders.create_if_needed(std::string(SHADERS_DIR), "triangle");
        God::shaders.create_if_needed(std::string(SHADERS_DIR), "point_as_sphere");
        CellAttribute<bool> visible_hex("visible", attr, hex, true);


        if (attr_active()){
            if (rt == RenderType::POINT_SPHERE){
                for (auto& it : attr.points) if (it.name == attr_name()){
                    point_renderer.color_map_prop = 1;
                    update();
                    return;
                }
            }
        }

        if (!attr_active()){
            rt = PLAIN;
            Triangles tri;
            FacetAttribute<bool> visible_tri(tri, true);
            FacetAttribute<int> primitive_id(tri, 0);
            CornerAttribute<float> value_tri(tri, 0);
            prepare_data_without_attribute(tri, visible_tri, value_tri, primitive_id);
            tri_renderer.update(tri, visible_tri, value_tri, primitive_id);
        }
        else {
            tri_renderer.color_map_prop = 1;
            show_object_id(attr_id);
            //rt = PLAIN;
            if (attr_primitive() == "cells") for (auto& it : attr.cells) if (it.name == attr_name())
                rt =RenderType::CELL;
                
            if (attr_primitive() == "facets")for (auto& it : attr.cell_facets) if (it.name == attr_name())
                rt = RenderType::FACET; 

            if (attr_primitive() == "corners")for (auto& it : attr.cell_corners) if (it.name == attr_name())
                if (rt!= RenderType::CORNER_GLYPH) rt = RenderType::CORNER;
            
            if (attr_primitive() == "points")for (auto& it : attr.points) if (it.name == attr_name())
                rt = RenderType::POINT;
            update();
        }
    }

    void HexahedraLayer::init(ObjectId obj){
        mesh = obj;
        tri_renderer.init();
        point_renderer.init();
        reset();
    }

    void HexahedraLayer::render(){
        if (!visible) return;
        if (rt==RenderType::POINT_SPHERE)
            point_renderer.render();
        else 
            tri_renderer.render();
    }


    void HexahedraLayer::destroy() {
        tri_renderer.destroy();
        point_renderer.destroy();
    }

    void HexahedraLayer::bind_attribute(ObjectId p_attr_id){
        attr_id = p_attr_id;
        reset();
    };







