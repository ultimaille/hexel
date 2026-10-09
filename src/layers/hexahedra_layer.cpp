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
        {
            bool tmp = render_as_spheres;
            if (attr_active() && attr_primitive() == "points")
                ImGui::Checkbox(label("Show vertices ", ""), &render_as_spheres);
            if (render_as_spheres != tmp) reset();
        }
        if (use_point_renderer())
            point_renderer.generate_gui(name);
        else 
            tri_renderer.generate_gui(name);
        Layer::generate_gui(name);
    }

    void HexahedraLayer::prepare_data_without_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    ){
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
    }

    template<class T>
    void HexahedraLayer::prepare_data_for_scalar_cell_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    ){
        CellAttribute<T> value_hex(attr_name, attr, hex);
        prepare_data_without_attribute(hex, visible_hex, tri, visible_tri, value_tri);
        for (auto h : tri.iter_halfedges()) value_tri[h] = value_hex[h / 36];
    }
    template<class T>
    void HexahedraLayer::prepare_data_for_scalar_facet_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    ){
        CellFacetAttribute<T> value_hex(attr_name, attr, hex);
        prepare_data_without_attribute(hex, visible_hex, tri, visible_tri, value_tri);
        for (auto h : tri.iter_halfedges()) value_tri[h] =  value_hex[h / 6];
    }
    template<class T>
    void HexahedraLayer::prepare_data_for_scalar_corner_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    ){
        CellCornerAttribute<T> value_hex(attr_name, attr, hex);
        prepare_data_without_attribute(hex, visible_hex, tri, visible_tri, value_tri);
        int map[2][3] = { {0,1,2},{2,3,0} };
        for (auto f : hex.iter_facets()) 
            FOR(t,2)FOR(v,3)value_tri[3*(2 * f+t)+v] = value_hex[f.corner(map[t][v])];
    }

    template<class T> 
    void HexahedraLayer::prepare_data_for_scalar_point_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    ){
        Log::add("::prepare_data_for_scalar_point_attribute(");
        PointAttribute<T> value_hex(attr_name, attr, hex);
        prepare_data_without_attribute(hex, visible_hex, tri, visible_tri, value_tri);
        int map[2][3] = { {0,1,2},{2,3,0} };
        for (auto f : hex.iter_facets())
            FOR(t, 2)FOR(v, 3)value_tri[3 * (2 * f + t)+v] = value_hex[f.vertex(map[t][v])];
    }

    template<class T> void HexahedraLayer::update_data_for_scalar_point_attribute_as_sphere(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name
    ){
        PointAttribute<T> value(attr_name, attr, hex);
        PointAttribute<float> value_float(hex);
        PointAttribute<bool> visible_pts(hex,false);
        FOR(v, hex.nverts()) value_float[v] = value[v];
        for (auto c : hex.iter_cells()) if (visible_hex[c]) FOR(lv,8)visible_pts[c.vertex(lv)];
        point_renderer.update(hex.points, visible_pts, value_float);
    }



    void HexahedraLayer::reset() {
        um_assert(mesh.ref() != std::nullopt);
        Hexahedra& hex = mesh;
        VolumeAttributes& attr = mesh;

        God::shaders.create_if_needed(std::string(SHADERS_DIR), "triangle");
        God::shaders.create_if_needed(std::string(SHADERS_DIR), "point_as_sphere");
        CellAttribute<bool> visible_hex("visible", attr, hex, true);

        
        if (attr_active()){
            if (attr_primitive() == "points" && render_as_spheres){
                for (auto& it : attr.points) if (it.name == attr_name()){
                    point_renderer.color_map_prop = 1;
                    if (attr_type() == "bool") update_data_for_scalar_point_attribute_as_sphere<bool>(hex, visible_hex, attr, it.name);
                    if (attr_type() == "char") update_data_for_scalar_point_attribute_as_sphere<char>(hex, visible_hex, attr, it.name);
                    if (attr_type() == "int") update_data_for_scalar_point_attribute_as_sphere<int>(hex, visible_hex, attr, it.name);
                    if (attr_type() == "float") update_data_for_scalar_point_attribute_as_sphere<float>(hex, visible_hex, attr, it.name);
                    if (attr_type() == "double") update_data_for_scalar_point_attribute_as_sphere<double>(hex, visible_hex, attr, it.name);
                    return;
                }
            }
        }



        Triangles tri;
        FacetAttribute<bool> visible_tri(tri, true);
        CornerAttribute<float> value_tri(tri, 0);

        //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name


        if (!attr_active())
            prepare_data_without_attribute(hex, visible_hex, tri, visible_tri, value_tri);
        else {
            tri_renderer.color_map_prop = 1;
            show_object_id(attr_id);
            if (attr_primitive() == "cells"){
                for (auto& it : attr.cells) if (it.name == attr_name()){
                    if (attr_type() == "bool") prepare_data_for_scalar_cell_attribute<bool>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "char") prepare_data_for_scalar_cell_attribute<char>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "int") prepare_data_for_scalar_cell_attribute<int>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "float") prepare_data_for_scalar_cell_attribute<float>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "double") prepare_data_for_scalar_cell_attribute<double>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                }
            }
            if (attr_primitive() == "facets"){
                for (auto& it : attr.cell_facets) if (it.name == attr_name()){
                    if (attr_type() == "bool") prepare_data_for_scalar_facet_attribute<bool>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "char") prepare_data_for_scalar_facet_attribute<char>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "int") prepare_data_for_scalar_facet_attribute<int>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "float") prepare_data_for_scalar_facet_attribute<float>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "double") prepare_data_for_scalar_facet_attribute<double>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                }
            }
            if (attr_primitive() == "corners"){
                for (auto& it : attr.cell_corners) if (it.name == attr_name()){
                    if (attr_type() == "bool") prepare_data_for_scalar_corner_attribute<bool>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "char") prepare_data_for_scalar_corner_attribute<char>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "int") prepare_data_for_scalar_corner_attribute<int>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "float") prepare_data_for_scalar_corner_attribute<float>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "double") prepare_data_for_scalar_corner_attribute<double>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                }
            }
            if (attr_primitive() == "points"){
                for (auto& it : attr.points) if (it.name == attr_name()){
                    if (attr_type() == "bool") prepare_data_for_scalar_point_attribute<bool>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "char") prepare_data_for_scalar_point_attribute<char>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "int") prepare_data_for_scalar_point_attribute<int>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "float") prepare_data_for_scalar_point_attribute<float>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                    if (attr_type() == "double") prepare_data_for_scalar_point_attribute<double>(hex, visible_hex, attr, it.name, tri, visible_tri, value_tri);
                }
            }
        }
        for (auto f : tri.iter_facets()) plop(visible_tri[f]);
        tri_renderer.update(tri, visible_tri, value_tri);
    }

    void HexahedraLayer::init(ObjectId obj){
        mesh = obj;
        tri_renderer.init();
        point_renderer.init();
        reset();
    }

    void HexahedraLayer::render(){
        if (!visible) return;
        if (use_point_renderer())
            point_renderer.render();
        else 
            tri_renderer.render();
    }

    int HexahedraLayer::primitive_id(int vertex_id) {
        // retrieve facet id from triangle num 
        // triangle num is equal to the provoking vertex_id
        // as there is 2 triangles per facet in hex =>
        return vertex_id / 2;
    }

    void HexahedraLayer::destroy() {
        tri_renderer.destroy();
        point_renderer.destroy();
    }

    void HexahedraLayer::bind_attribute(ObjectId p_attr_id){
        attr_id = p_attr_id;
        reset();
    };







