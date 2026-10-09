#include "basic_layers.h"
#include "core/layers.h"
#include "core/camera.h"
#include "core/picker.h"
#include "core/xcf.h"
#include "core/render_target.h"
#include "core/window.h"
#include "core/panels.h"
#include "core/layers.h"
#include "core/shaders.h"
#include "core/mode.h"

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
            value[h] = h.from().pos()[0];
        }

        FacetAttribute<bool> visible(true);
        visible.bind("visible", attr, tri);

        primitive_renderer.init();
        primitive_renderer.update(tri, visible, value);
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








    QuadsLayer::QuadsLayer()  { primitive_renderer.layer_id = id; }

    void QuadsLayer::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        Layer::generate_gui(name);
    }

    void QuadsLayer::reset() {
        // TODO free vba/vbo/texture
        init(mesh);
    }


    void QuadsLayer::init(ObjectId obj){
        mesh = obj;
        um_assert(obj.ref() != std::nullopt);
        Quads& quads = obj;
        SurfaceAttributes& attr = obj;

            God::shaders.create_if_needed(std::string(SHADERS_DIR), "triangle");

        CornerAttribute<float> value(quads);
        for (auto h : quads.iter_halfedges())  {
            value[h] = h.from().pos()[0];
        }

        FacetAttribute<bool> visible(true);
        visible.bind("visible", attr, quads);

        primitive_renderer.init();
        primitive_renderer.update(quads, visible, value);
    }

    void QuadsLayer::render(){
        if (visible)
            primitive_renderer.render();
    }

    int QuadsLayer::primitive_id(int vertex_id) {
        // retrieve facet id from triangle num 
        // triangle num is equal to the provoking vertex_id
        // as there is 4 triangles per facet in quad =>
        return vertex_id / 4;
    }

    void QuadsLayer::destroy() {
        primitive_renderer.destroy();
    }




    
    TetrahedraLayer::TetrahedraLayer()  { primitive_renderer.layer_id = id; }

    void TetrahedraLayer::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        Layer::generate_gui(name);
    }

    void TetrahedraLayer::reset() {
        // TODO free vba/vbo/texture
        init(mesh);
    }

    void TetrahedraLayer::init(ObjectId obj){
        mesh = obj;
        um_assert(obj.ref() != std::nullopt);
        Tetrahedra& tet = obj;
        VolumeAttributes& attr = obj;

      
            God::shaders.create_if_needed(std::string(SHADERS_DIR), "triangle");

        CellCornerAttribute<float> value(tet);
        int n = tet.nhalfedges();
        int n2 = value.ptr->data.size();
        for (auto h : tet.iter_corners())  {
            value[h] = static_cast<float>(((vec3)h.vertex()).x);
        }

        CellAttribute<bool> visible("visible", attr, tet, true);

        primitive_renderer.init();
        primitive_renderer.update(tet, visible, value);
    }

    void TetrahedraLayer::render(){
        if (visible)
            primitive_renderer.render();
    }

    int TetrahedraLayer::primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void TetrahedraLayer::destroy() {
        primitive_renderer.destroy();
    }






    PointSetLayer::PointSetLayer() { primitive_renderer.layer_id = id; }

    void PointSetLayer::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        Layer::generate_gui(name);
    }

    void PointSetLayer::init(ObjectId obj){
        mesh = obj;
        PointSet& ps = obj;
        PointSetAttributes& attr = obj;

        PointAttribute<float> value(ps);
        FOR(v, ps.size()){
            value[v] = ps[v][0];
        }

        PointAttribute<bool> visible("visible", attr, ps, true);

        primitive_renderer.init();
        primitive_renderer.update(ps, visible, value);
    }

    void PointSetLayer::render(){
        if (visible)primitive_renderer.render();
    }

    void PointSetLayer::destroy() {
        primitive_renderer.destroy();
    }






    PolyLineLayer::PolyLineLayer()  { primitive_renderer.layer_id = id; }

    void PolyLineLayer::generate_gui(std::string name) {
        primitive_renderer.generate_gui(name);
        Layer::generate_gui(name);
    }


    void PolyLineLayer::init(ObjectId obj){
        mesh = obj;
        PolyLine& pl = obj;
        PolyLineAttributes& attr = obj;

        PointAttribute<float> value(pl, 0);
        for (auto v : pl.iter_vertices())
            value[v] = v.pos()[0];

        EdgeAttribute<bool> visible("visible", attr, pl, true);

        primitive_renderer.init();
        primitive_renderer.push(pl, visible, value);
    }

    void PolyLineLayer::render(){
        if (visible)primitive_renderer.render();
    }

    int PolyLineLayer::primitive_id(int vertex_id) {
        return vertex_id; // edge id from vertex id
    }

    void PolyLineLayer::destroy() {
        primitive_renderer.destroy();
    }








