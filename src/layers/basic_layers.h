#include "basic_renderers.h"


struct RenderLambertTriangles: public RenderLayer{
	std::string mm_name;
	std::string triangle_name;
	TriangleRenderer primitive_renderer;

	RenderLambertTriangles() : primitive_renderer{_id} {}

	void generate_gui(std::string name){
		primitive_renderer.generate_gui(name);
	}

	bool handle(Event event) {return true;}

	void init(std::string mm,std::string triangle){
		triangle_name = triangle;
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		um_assert(God::xcf[mm_name].triangles.contains(triangle_name));
		Triangles&  tri = God::xcf[mm_name].triangles[triangle_name].mesh;

		if(!God::shaders.contains("triangle"))
			God::shaders.add(std::string(SHADERS_DIR),"triangle");

		CornerAttribute<float> value(tri);
		for(auto h:tri.iter_halfedges())  {
			value[h] = h.from().pos()[0];
			//if(h.from().pos().x>0) value[h]  = -1;
		}
		// here multiple overload of the same function for different types of attributes ?
		// ou on mappe les attributs sur un corner attribute ?
		primitive_renderer.init_from_mesh(tri,value);
	}

	void render(){
		primitive_renderer.render();
	}

	virtual int primitive_id(int vertex_id) {
		return vertex_id; // triangle id from vertex id
	}

	void destroy() {
		primitive_renderer.destroy();
	}

};

struct RenderSpheres: public RenderLayer{
	std::string mm_name;
	PointRenderer primitive_renderer;

	RenderSpheres() : primitive_renderer{_id} {}

	void generate_gui(std::string name){
		primitive_renderer.generate_gui("name");
	}

	bool handle(Event event) { 
		if (event.event_type == Event::MOUSE_PRESSED) {			
			Picker picker;
			auto [layer_id, primitive_id] = picker.at({God::mouse.x, God::mouse.y});
			Log::add("layer id: " + std::to_string(layer_id));
			Log::add("primitive id: " + std::to_string(primitive_id));
		}
		return true; 
	}

	void init(std::string mm){
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		PointSet&  ps = God::xcf[mm_name].pointset;
		God::shaders.add(std::string(SHADERS_DIR),"point_as_sphere");

		PointAttribute<float> value(ps);
		FOR(v,ps.size()){
			value[v] = ps[v][0];
			//if(ps[v][0]>0) value[v] = -1;
		}
		primitive_renderer.init_from_mesh(ps,value);
	}

	void render(){
		primitive_renderer.render();
	}

	void destroy() {
		primitive_renderer.destroy();
	}
};





struct RenderTubes: public RenderLayer{
	std::string mm_name;
	std::string polyline_name;
	SegmentRenderer primitive_renderer;

	RenderTubes() : primitive_renderer{_id} {}

	void generate_gui(std::string name){
		primitive_renderer.generate_gui(name);
	}

	bool handle(Event event) { return true; }

	void init(std::string mm,std::string polyline){
		polyline_name = polyline;
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		um_assert(God::xcf[mm_name].polylines.contains(polyline_name));
		PolyLine&  pl= God::xcf[mm_name].polylines[polyline_name].mesh;

		PointAttribute<float> value(pl,0);
		for(auto v:pl.iter_vertices()) {
			value[v]= v.pos()[0];
			//if(v.pos().x>0) value[v] = -1;
		}
		primitive_renderer.init_from_mesh(pl,value);
	}

	void render(){
		primitive_renderer.render();
	}

	virtual int primitive_id(int vertex_id) {
		return vertex_id; // edge id from vertex id
	}

	void destroy() {
		primitive_renderer.destroy();
	}
};








