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
        FacetAttribute<int> primitive_id(tri);
        for (auto f : tri.iter_facets()) primitive_id[f] = f;
        primitive_renderer.init();
        primitive_renderer.update(tri, visible, value,primitive_id);
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
        PointAttribute<int> primitive_id(ps);
        FOR(v, ps.size())
            primitive_id[v] = v;

        primitive_renderer.init();
        primitive_renderer.update(ps, visible, value, primitive_id);
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
        EdgeAttribute<int> primitive_id(pl);
        for (auto e : pl.iter_edges())
            primitive_id[e] = e;
        primitive_renderer.init();
        primitive_renderer.push(pl, visible, value, primitive_id);
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








