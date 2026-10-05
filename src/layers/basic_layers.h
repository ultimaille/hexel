#pragma once 
#include "basic_renderers.h"


struct RenderLambertTriangles : public RenderLayer{
    TriangleRenderer primitive_renderer;

    RenderLambertTriangles() : primitive_renderer{ _id } {}

    void generate_gui(std::string name){
        //if (ImGui::TreeNode((name).c_str())) {
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
        //	ImGui::TreePop();
        //}
    }

    bool handle(Event event) { 
        if (event.who == ObjectId(events::MOUSE) && God::mouse.clicked(GLFW_MOUSE_BUTTON_LEFT)) {
            auto l = God::layers.find_by_id(3);
            bool old_val = l.value().get().visible;
            l.value().get().visible = false;
            Picker picker;

            auto [layer_id, primitive_id, object_id] = picker.at({ God::mouse.current.x, God::mouse.current.y });
            Log::add("layer id: " + std::to_string(layer_id));
            Log::add("primitive id: " + std::to_string(primitive_id));
            // Triangles &t = object_id;
            // SurfaceAttributes &s = object_id;
            l.value().get().visible = old_val;

        }
        return true;
    }
    bool require(ObjectId object) {
        return object == _mesh;
    }
    void reset() {
        init(_mesh);
    }


    void init(ObjectId obj){
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
        // here multiple overload of the same function for different types of attributes ?
        // ou on mappe les attributs sur un corner attribute ?
        primitive_renderer.init_from_mesh(tri, value);
    }

    void render(){
        if (visible)
            primitive_renderer.render();
    }

    virtual int primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void destroy() {
        primitive_renderer.destroy();
    }

};

struct RenderLambertQuads : public RenderLayer {
    TriangleRenderer primitive_renderer;

    RenderLambertQuads() : primitive_renderer{ _id } {}

    void generate_gui(std::string name){
        //if (ImGui::TreeNode((name).c_str())) {
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
        //	ImGui::TreePop();
        //}
    }

    bool handle(Event event) { return true; }
    bool require(ObjectId object) {
        return object == _mesh;
    }
    void reset() {
        // TODO free vba/vbo/texture
        init(_mesh);
    }


    void init(ObjectId obj){
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
        // here multiple overload of the same function for different types of attributes ?
        // ou on mappe les attributs sur un corner attribute ?
        primitive_renderer.init_from_mesh(quads, value);
    }

    void render(){
        if (visible)
            primitive_renderer.render();
    }

    virtual int primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void destroy() {
        primitive_renderer.destroy();
    }

};

struct RenderLambertTet : public RenderLayer {

    TriangleRenderer primitive_renderer;

    RenderLambertTet() : primitive_renderer{ _id } {}

    void generate_gui(std::string name){
        //if (ImGui::TreeNode((name).c_str())) {
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
        //	ImGui::TreePop();
        //}
    }

    bool handle(Event event) { return true; }
    bool require(ObjectId object) {
        return object == _mesh;
    }
    void reset() {
        // TODO free vba/vbo/texture
        init(_mesh);
    }


    void init(ObjectId obj){
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
        // here multiple overload of the same function for different types of attributes ?
        // ou on mappe les attributs sur un corner attribute ?
        primitive_renderer.init();
        primitive_renderer.push(tet, value);
    }

    void render(){
        if (visible)
            primitive_renderer.render();
    }

    virtual int primitive_id(int vertex_id) {
        return vertex_id; // triangle id from vertex id
    }

    void destroy() {
        primitive_renderer.destroy();
    }

};

struct RenderSpheres : public RenderLayer{
    PointRenderer primitive_renderer;

    RenderSpheres() : primitive_renderer{ _id } {}

    void generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool handle(Event event) {
        return true;
    }
    bool require(ObjectId object) {
        return object == _mesh;
    }


    void init(ObjectId obj){
        _mesh = obj;
        PointSet& ps = obj;
        PointSetAttributes& attr = obj;

        PointAttribute<float> value(ps);
        FOR(v, ps.size()){
            value[v] = ps[v][0];
        }
        primitive_renderer.init_from_mesh(ps, value);
    }

    void render(){
        if (visible)primitive_renderer.render();
    }

    void destroy() {
        primitive_renderer.destroy();
    }
};





struct RenderTubes : public RenderLayer{
    SegmentRenderer primitive_renderer;

    RenderTubes() : primitive_renderer{ _id } {}

    void generate_gui(std::string name) {
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool handle(Event event) { return true; }
    bool require(ObjectId object) {
        return object == _mesh;
    }


    void init(ObjectId obj){
        _mesh = obj;
        PolyLine& pl = obj;
        PolyLineAttributes& attr = obj;

        PointAttribute<float> value(pl, 0);
        for (auto v : pl.iter_vertices())
            value[v] = v.pos()[0];
        primitive_renderer.init_from_mesh(pl, value);
    }

    void render(){
        if (visible)primitive_renderer.render();
    }

    virtual int primitive_id(int vertex_id) {
        return vertex_id; // edge id from vertex id
    }

    void destroy() {
        primitive_renderer.destroy();
    }
};








