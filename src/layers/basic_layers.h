#include "layers/basic_renderers.h"








struct RenderLambertTriangles: public RenderLayer{
	std::string mm_name;
	std::string triangle_name;
	TriangleRenderer primitive_renderer;

	RenderLambertTriangles(){}

	void generate_gui(std::string name){
		primitive_renderer.generate_gui("name");
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
			if(h.from().pos().x>0) value[h]  = -1;
		}
		primitive_renderer.init_from_mesh(tri,value);

	}
	void render(){
		primitive_renderer.render();
	}
};












struct RenderSpheres: public RenderLayer{

	std::string mm_name;
	PointRenderer primitive_renderer;


	void generate_gui(std::string name){
		primitive_renderer.generate_gui("name");

		//ImGui::ColorEdit3(("MyColor##"+name).c_str(),(float*)&pts_renderer.color,ImGuiColorEditFlags_None);
	}

	bool handle(Event event) { return true; }


	void init(std::string mm){
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		PointSet&  ps = God::xcf[mm_name].pointset;
		God::shaders.add(std::string(SHADERS_DIR),"point_as_sphere");

		PointAttribute<float> value(ps);
		FOR(v,ps.size()){
			value[v] = ps[v][0];
			if(ps[v][0]>0) value[v] = -1;

		}
		primitive_renderer.init_from_mesh(ps,value);
	}


	void render(){
		primitive_renderer.render();
	}
};














struct RenderTubes: public RenderLayer{

	std::string mm_name;
	std::string polyline_name;


	SegmentRenderer primitive_renderer;


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
			if(v.pos().x>0) value[v] = -1;
		}
		primitive_renderer.init_from_mesh(pl,value);
	}


	void render(){
		primitive_renderer.render();
	}
};








