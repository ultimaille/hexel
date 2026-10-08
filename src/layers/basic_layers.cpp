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


RenderLambertTriangles::RenderLambertTriangles() : primitive_renderer{ _id } {}

    void RenderLambertTriangles::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool RenderLambertTriangles::handle(Event event) {
        if (event.who == ObjectId(events::MOUSE) && !ImGui::GetIO().WantCaptureMouse && God::mouse.clicked(GLFW_MOUSE_BUTTON_LEFT)) {
            Picker picker({God::mouse.current.x, God::mouse.current.y, 1, 1});
            auto [layer_id, primitive_id, object_id] = picker.at({ God::mouse.current.x, God::mouse.current.y });
            Log::add("layer id: " + std::to_string(layer_id));
            Log::add("primitive id: " + std::to_string(primitive_id));
        }
        return true;
    }

    void RenderLambertTriangles::reset() {
        init(_mesh);
    }


    void RenderLambertTriangles::init(ObjectId obj){
        _mesh = obj;
        um_assert(obj.ref() != std::nullopt);
        Triangles& tri = obj;
        SurfaceAttributes& attr = obj;

        if (!God::shaders.contains("triangle"))
            God::shaders.add(std::string(SHADERS_DIR), "triangle");

        CornerAttribute<float> value(tri);
        for (auto h : tri.iter_halfedges())  {
            value[h] = h.from().pos()[0];
        }

        FacetAttribute<bool> visible(true);
        visible.bind("visible", attr, tri);

        primitive_renderer.init();
        primitive_renderer.update(tri, visible, value);
    }

    void RenderLambertTriangles::render(){
        if (visible)
            primitive_renderer.render();
    }

    int RenderLambertTriangles::primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void RenderLambertTriangles::destroy() {
        primitive_renderer.destroy();
    }








    RenderLambertQuads::RenderLambertQuads() : primitive_renderer{ _id } {}

    void RenderLambertQuads::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool RenderLambertQuads::handle(Event event) { return true; }

    void RenderLambertQuads::reset() {
        // TODO free vba/vbo/texture
        init(_mesh);
    }


    void RenderLambertQuads::init(ObjectId obj){
        _mesh = obj;
        um_assert(obj.ref() != std::nullopt);
        Quads& quads = obj;
        SurfaceAttributes& attr = obj;

        if (!God::shaders.contains("triangle"))
            God::shaders.add(std::string(SHADERS_DIR), "triangle");

        CornerAttribute<float> value(quads);
        for (auto h : quads.iter_halfedges())  {
            value[h] = h.from().pos()[0];
        }

        FacetAttribute<bool> visible(true);
        visible.bind("visible", attr, quads);

        primitive_renderer.init();
        primitive_renderer.update(quads, visible, value);
    }

    void RenderLambertQuads::render(){
        if (visible)
            primitive_renderer.render();
    }

    int RenderLambertQuads::primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void RenderLambertQuads::destroy() {
        primitive_renderer.destroy();
    }




    
    RenderLambertTet::RenderLambertTet() : primitive_renderer{ _id } {}

    void RenderLambertTet::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool RenderLambertTet::handle(Event event) { return true; }

    void RenderLambertTet::reset() {
        // TODO free vba/vbo/texture
        init(_mesh);
    }


    void RenderLambertTet::init(ObjectId obj){
        _mesh = obj;
        um_assert(obj.ref() != std::nullopt);
        Tetrahedra& tet = obj;
        VolumeAttributes& attr = obj;

        if (!God::shaders.contains("triangle"))
            God::shaders.add(std::string(SHADERS_DIR), "triangle");

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

    void RenderLambertTet::render(){
        if (visible)
            primitive_renderer.render();
    }

    int RenderLambertTet::primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void RenderLambertTet::destroy() {
        primitive_renderer.destroy();
    }




    RenderLambertHex::RenderLambertHex() : primitive_renderer{ _id } {}

    void RenderLambertHex::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool RenderLambertHex::handle(Event event) { return true; }

    void RenderLambertHex::reset() {
        init(_mesh);
    }

    void RenderLambertHex::init(ObjectId obj){
        _mesh = obj;
        um_assert(obj.ref() != std::nullopt);
        Hexahedra& hex = obj;
        VolumeAttributes& attr = obj;

        if (!God::shaders.contains("triangle"))
            God::shaders.add(std::string(SHADERS_DIR), "triangle");
            
        CellAttribute<bool> visible_hex("visible", attr, hex, true);

        Triangles tri;
        tri.points.create_points(hex.ncorners());
        tri.create_facets(2*hex.nfacets());
        for (auto c : hex.iter_corners()) tri.points[c] = c.vertex().pos();
        int shrink=1;
        double w = double(shrink) / 10.;
        for (auto c : hex.iter_cells()){
            vec3 G = Hexahedron(c).bary_verts();
            //FOR(lv,8)
            for (auto v : c.iter_corners())
                 tri.points[v] = (1. - w) * tri.points[v] + w * G;
        }
            for (auto f : hex.iter_facets()) {
                tri.vert(2 * f, 0) = f.corner(0);
                tri.vert(2 * f, 1) = f.corner(1);
                tri.vert(2 * f, 2) = f.corner(2);
                tri.vert(2 * f + 1, 0) = f.corner(2);
                tri.vert(2 * f + 1, 1) = f.corner(3);
                tri.vert(2 * f + 1, 2) = f.corner(0);
            }
        FacetAttribute<bool> visible_tri(tri, true);
        CornerAttribute<float> value_tri(tri,0);
        primitive_renderer.init();
        primitive_renderer.update(tri, visible_tri, value_tri);
    }

    void RenderLambertHex::render(){
        if (visible)
            primitive_renderer.render();
    }

    int RenderLambertHex::primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void RenderLambertHex::destroy() {
        primitive_renderer.destroy();
    }








    RenderSpheres::RenderSpheres() : primitive_renderer{ _id } {}

    void RenderSpheres::generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool RenderSpheres::handle(Event event) {
        return true;
    }

    void RenderSpheres::init(ObjectId obj){
        _mesh = obj;
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

    void RenderSpheres::render(){
        if (visible)primitive_renderer.render();
    }

    void RenderSpheres::destroy() {
        primitive_renderer.destroy();
    }






    RenderTubes::RenderTubes() : primitive_renderer{ _id } {}

    void RenderTubes::generate_gui(std::string name) {
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool RenderTubes::handle(Event event) { return true; }


    void RenderTubes::init(ObjectId obj){
        _mesh = obj;
        PolyLine& pl = obj;
        PolyLineAttributes& attr = obj;

        PointAttribute<float> value(pl, 0);
        for (auto v : pl.iter_vertices())
            value[v] = v.pos()[0];

        EdgeAttribute<bool> visible("visible", attr, pl, true);

        primitive_renderer.init();
        primitive_renderer.push(pl, visible, value);
    }

    void RenderTubes::render(){
        if (visible)primitive_renderer.render();
    }

    int RenderTubes::primitive_id(int vertex_id) {
        return vertex_id; // edge id from vertex id
    }

    void RenderTubes::destroy() {
        primitive_renderer.destroy();
    }








