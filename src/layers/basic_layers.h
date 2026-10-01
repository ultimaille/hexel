#pragma once 
#include "basic_renderers.h"


struct RenderLambertTriangles : public RenderLayer{
    ObjectId mesh;
    TriangleRenderer primitive_renderer;

    RenderLambertTriangles() : primitive_renderer{ _id } {}

    void generate_gui(std::string name){
        //if (ImGui::TreeNode((name).c_str())) {
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
        //	ImGui::TreePop();
        //}
    }

    bool handle(Event event) { return true; }
    bool require(ObjectId object) {
        return object == mesh;
    }
    void reset() {
        // TODO free vba/vbo/texture
        init(mesh);
    }


    void init(ObjectId obj){
        mesh = obj;
        um_assert(obj.ptr() != NULL);
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

struct RenderSpheres : public RenderLayer{
    ObjectId mesh;
    PointRenderer primitive_renderer;

    RenderSpheres() : primitive_renderer{ _id } {}

    void generate_gui(std::string name){
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool handle(Event event) {
        if (event.who == ObjectId({ chunk_mouse }) && God::mouse.clicked(GLFW_MOUSE_BUTTON_LEFT)) {
            Picker picker;
            auto [layer_id, primitive_id] = picker.at({ God::mouse.current.x, God::mouse.current.y });
            Log::add("layer id: " + std::to_string(layer_id));
            Log::add("primitive id: " + std::to_string(primitive_id));
        }
        return true;
    }
    bool require(ObjectId object) {
        return object == mesh;
    }


    void init(ObjectId obj){
        mesh = obj;
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
    ObjectId mesh;
    SegmentRenderer primitive_renderer;

    RenderTubes() : primitive_renderer{ _id } {}

    void generate_gui(std::string name) {
        primitive_renderer.generate_gui(name);
        RenderLayer::generate_gui(name);
    }

    bool handle(Event event) { return true; }
    bool require(ObjectId object) {
        return object == mesh;
    }


    void init(ObjectId obj){
        mesh = obj;
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








