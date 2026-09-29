#include "basic_renderers.h"


struct RenderLambertTriangles: public RenderLayer{
	ObjectId mesh;
	TriangleRenderer primitive_renderer;

	RenderLambertTriangles() : primitive_renderer{_id} {}

	void generate_gui(std::string name){
		if (ImGui::TreeNode((name).c_str())) {
			primitive_renderer.generate_gui(name);
			ImGui::Checkbox(("visible##" + name).c_str(), &visible);
			ImGui::TreePop();
		}


	}

	bool handle(Event event) {return true;}
	bool require(ObjectId object) {
		return object==mesh;
	}
	void reset() {
		// TODO free vba/vbo/texture
		init(mesh);
	}

	
	void init(ObjectId obj){
		mesh = obj;
		um_assert(obj.ptr() != NULL);
		auto& [tri, attr] = *((MultiMesh::MeshAttr<Triangles, SurfaceAttributes>*) obj.ptr());

		if(!God::shaders.contains("triangle"))
			God::shaders.add(std::string(SHADERS_DIR),"triangle");

		CornerAttribute<float> value(tri);
		for(auto h:tri.iter_halfedges())  {
			value[h] = h.from().pos()[0];
		}
		primitive_renderer.init_from_mesh(tri,value);
	}

	void render(){
		if(visible)
		primitive_renderer.render();
	}

	virtual int primitive_id(int vertex_id) {
		return vertex_id / 3; // triangle id from vertex id
	}

};

struct RenderSpheres: public RenderLayer{
	ObjectId mesh;
	PointRenderer primitive_renderer;

	RenderSpheres() : primitive_renderer{_id} {}

	void generate_gui(std::string name){
		if (ImGui::TreeNode((name).c_str())) {
			primitive_renderer.generate_gui(name);
			ImGui::Checkbox(("visible##" + name).c_str(), &visible);
			ImGui::TreePop();
		}


	}

	bool handle(Event event) { 
		if (event.who == ObjectId({ chunk_mouse }) && God::mouse.clicked(0)) {
			Picker picker;
			auto [layer_id, primitive_id] = picker.at({God::mouse.current_state.x, God::mouse.current_state.y});
			Log::add("layer id: " + std::to_string(layer_id));
			Log::add("primitive id: " + std::to_string(primitive_id));
		}
		return true; 
	}
	bool require(ObjectId object) {
		return object==mesh;
	}


	void init(ObjectId obj){
		mesh = obj;
		auto& [ps, attr] = *((MultiMesh::MeshAttr<PointSet, PointSetAttributes>*) obj.ptr());

		God::shaders.add(std::string(SHADERS_DIR),"point_as_sphere");

		PointAttribute<float> value(ps);
		FOR(v,ps.size()){
			value[v] = ps[v][0];
		}
		primitive_renderer.init_from_mesh(ps,value);
	}

	void render(){
		if (visible)primitive_renderer.render();
	}
};





struct RenderTubes: public RenderLayer{
	ObjectId mesh;
	SegmentRenderer primitive_renderer;

	RenderTubes() : primitive_renderer{_id} {}

	void generate_gui(std::string name) {
		if (ImGui::TreeNode((name).c_str())) {
			primitive_renderer.generate_gui(name);
			ImGui::Checkbox(("visible##" + name).c_str(), &visible);
			ImGui::TreePop();
		}
	}

	bool handle(Event event) { return true; }
	bool require(ObjectId object) {
		return object==mesh;
	}


	void init(ObjectId obj){
		mesh = obj;
		auto& [pl, attr] = *((MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>*) obj.ptr());

		PointAttribute<float> value(pl,0);
		for(auto v:pl.iter_vertices()) 
			value[v]= v.pos()[0];
		primitive_renderer.init_from_mesh(pl,value);
	}

	void render(){
		if (visible)primitive_renderer.render();
	}

	virtual int primitive_id(int vertex_id) {
		return vertex_id / 2; // edge id from vertex id
	}
};








