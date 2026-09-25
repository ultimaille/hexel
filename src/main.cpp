#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <ultimaille/all.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdlib>
#include <iostream>

#include <string>

#include <ultimaille/all.h>
#include "core/core.h"




// -------------------------------------------------------------------------------
//                                    Examples of optional GUI windows
// -------------------------------------------------------------------------------


struct XCFViewer: public Panel {
	void generate_gui(){
		ImGui::SetNextWindowPos(ImVec2(10,10),ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(150,200),ImGuiCond_Always);


		ImGui::Begin("XCFViewer",nullptr,ImGuiWindowFlags_AlwaysAutoResize);


		std::vector<std::string> mm_to_kill;
		for(auto &[name,obj]:God::xcf){
			if(ImGui::TreeNode(name.c_str())){
				if(ImGui::Button("Delete MultiMesh"))
					mm_to_kill.push_back(name);
				ImGui::TreePop();
			}
		}
		for(auto name:mm_to_kill){
			God::xcf.erase(name);
		}
		ImGui::End();
	}
};
struct LayerViewer: public Panel {
	void generate_gui(){
		ImGui::SetNextWindowPos(ImVec2(10,210),ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(150,400),ImGuiCond_Always);
		ImGui::Begin("LayersConfig",nullptr,ImGuiWindowFlags_AlwaysAutoResize);

		for(auto &[name,obj]:God::layers){
			if(ImGui::TreeNode(name.c_str())){
				obj->generate_gui(name);
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
};


// -------------------------------------------------------------------------------
//                                    Example of a layer
// -------------------------------------------------------------------------------

struct RenderLambertTriangles: public RenderLayer{
	GLuint VAO,VBO,EBO;
	int ncorners;
	float color[3];

	std::string mm_name;
	std::string triangle_name;

	RenderLambertTriangles(){
		FOR(i,3) color[i] = 1;
	}

	void generate_gui(std::string name){
		ImGui::ColorEdit3(("MyColor##"+name).c_str(),(float*)&color,ImGuiColorEditFlags_None);
	}

	bool handle(Event event){
		//	if (!God::xcf[mm_name].triangles[triangle_name].modified) return true; // TODO: obsolete, à refaire avec la newsletter
		// Log::add("need to update vbo");
		return true;
	}

	// bool is_cleanup_ready() override {
	// 	return !God::xcf.contains(mm_name) || God::xcf[mm_name].triangles.contains(triangle_name);
	// }

	void init(std::string mm,std::string triangle){
		triangle_name = triangle;
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		um_assert(God::xcf[mm_name].triangles.contains(triangle_name));
		Triangles&  tri = God::xcf[mm_name].triangles[triangle_name].mesh;

		if (!God::shaders.contains("lamberttri"))
			God::shaders.add(std::string(SHADERS_DIR),"lamberttri");

		ncorners = tri.ncorners();
		BBox3 box;
		for(auto v:tri.iter_vertices()) box.add(v.pos());

		std::vector<unsigned int> indices(3*tri.nfacets());
		for(auto h:tri.iter_halfedges()) indices[h] = h.from();

		std::vector<float> vertices(6*tri.nverts(),0);
		for(auto v:tri.iter_vertices()){
			FOR(d,3) vertices[6*v+d] = 2.*(v.pos()-box.center())[d]/box.size().norm();//v.pos()[d];
			vertices[6*v+5] = 1;
		}



		glGenVertexArrays(1,&VAO);
		glGenBuffers(1,&VBO);
		glGenBuffers(1,&EBO);

		glBindVertexArray(VAO);



		// Sommets
		glBindBuffer(GL_ARRAY_BUFFER,VBO);
		glBufferData(GL_ARRAY_BUFFER,sizeof(float)*vertices.size(),vertices.data(),GL_STATIC_DRAW);

		// Indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(unsigned int )*indices.size(),indices.data(),GL_STATIC_DRAW);

		// position
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6 * sizeof(float),(void*)0);
		glEnableVertexAttribArray(0);

		// normale
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6 * sizeof(float),(void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);
	}

	void render(){
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

		GLuint shaderProgram=God::shaders["lamberttri"];
		glUseProgram(shaderProgram);

		glBindVertexArray(VAO);
		const float model[16] = {1,0,0,0 ,0,1,0,0, 0,0,1,0 ,0,0,0,1};
////	auto [w,h] = God::context.screen_size();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"), 1, GL_TRUE, God::camera.projection());
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_TRUE,God::camera.view());
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_TRUE,model);

		glUniform3f(glGetUniformLocation(shaderProgram,"lightPos"),1.0f,1.0f,0.0f);
		glUniform3f(glGetUniformLocation(shaderProgram,"lightColor"),0.9f,0.5f,0.5f);
		glUniform3f(glGetUniformLocation(shaderProgram,"objectColor"),color[0],color[1],color[2]);


		glDrawElements(GL_TRIANGLES,ncorners,GL_UNSIGNED_INT,nullptr);
		glBindVertexArray(0);
	}
};







bool no_gl_error() {
	switch(glGetError()){
	case GL_INVALID_ENUM:					Log::error("GL_INVALID_ENUM"); return false;
	case GL_INVALID_VALUE:					Log::error("GL_INVALID_VALUE"); return false;
	case GL_INVALID_OPERATION:				Log::error("GL_INVALID_OPERATION"); return false;
	case GL_INVALID_FRAMEBUFFER_OPERATION:	Log::error("GL_INVALID_FRAMEBUFFER_OPERATION"); return false;
	case GL_OUT_OF_MEMORY:					Log::error("GL_OUT_OF_MEMORY"); return false;
	}
	return true;
}






struct Renderer{
	GLuint shaderProgram;
	float light_direction[3] = {1,1,1};


	void declare_projection_matrix(){
		auto [w,h] = God::context.screen_size();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_TRUE,God::camera.projection());
	}
	void declare_inv_projection_matrix(){
		auto [w,h] = God::context.screen_size();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"inv_projection"),1,GL_TRUE,God::camera.inverse_projection());
	}
	void declare_view_matrix(){
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_TRUE,God::camera.view());
	}
	void declare_model_matrix(){
		float model[16] = {1,0,0,0 ,0,1,0,0, 0,0,1,0 ,0,0,0,1};
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_TRUE,model);
	}
	void declare_light_direction(){
		glUniform3fv(glGetUniformLocation(shaderProgram,"light_direction"),1,light_direction);
	}
	void declare_viewport(){
		auto [w,h] = God::context.screen_size();
		glUniform2f(glGetUniformLocation(shaderProgram,"viewport"),float(w),float(h));
	}
};


struct PointRenderer : public Renderer{
	GLuint vao,vbo;
	int npts;


	float color[3] = {.5,1,.5};
	int radius_in_pixel=5;


		void init(float* pts,int pts_size){
		npts = pts_size/3;
		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER,pts_size * sizeof(float),pts,GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,nullptr);
		glBindVertexArray(0);
		um_assert(no_gl_error());
	}




	void render(){
		um_assert(no_gl_error());
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_PROGRAM_POINT_SIZE);
		shaderProgram=God::shaders["point_as_sphere"];
		glUseProgram(shaderProgram);

		declare_projection_matrix();
		declare_inv_projection_matrix();
		declare_view_matrix();
		declare_model_matrix();
		declare_light_direction();
		declare_viewport();

		auto [w,h] = God::context.screen_size();

		float pointRadius = 2.*double(radius_in_pixel)/(God::camera.impl->projection_matrix()[1][1]*double(h));
		glUniform1f(glGetUniformLocation(shaderProgram,"R"),pointRadius);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);
		glBindVertexArray(vao);

		glDrawArrays(GL_POINTS,0,GLsizei(npts));
		glBindVertexArray(0);

		um_assert(no_gl_error());
	}

};


struct RenderSpheres: public RenderLayer{

	std::string mm_name;
	std::string triangle_name;


	PointRenderer pts_renderer;


	void generate_gui(std::string name){
		ImGui::ColorEdit3(("MyColor##"+name).c_str(),(float*)&pts_renderer.color,ImGuiColorEditFlags_None);
	}

	bool handle(Event event){
		if(!God::xcf.contains(mm_name)) return false;
		if(!God::xcf[mm_name].triangles.contains(triangle_name)) return false;
		return true;
	}

	void init(std::string mm,std::string triangle){
		triangle_name = triangle;
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		um_assert(God::xcf[mm_name].triangles.contains(triangle_name));
		Triangles&  tri = God::xcf[mm_name].triangles[triangle_name].mesh;
		God::shaders.add(std::string(SHADERS_DIR),"point_as_sphere");
		BBox3 box;
		for(auto v:tri.iter_vertices()) box.add(v.pos());

		//npts = tri.nverts();
		std::vector<float> vertices(3*tri.nverts(),0);
		for(auto v:tri.iter_vertices()){
			FOR(d,3) vertices[3*v+d] = 2.*(v.pos()-box.center())[d]/box.size().norm();
		}
		pts_renderer.init(vertices.data(),vertices.size());
	}


	void render(){
		pts_renderer.render();
	}
};














struct SegmentRenderer: public Renderer{
	GLuint vao,vbo;
	int nsegments;


	float color[3] = {.5,1,.5};
	int radius_in_pixel=100;


	void init(float* pts,int nedges){
		God::shaders.add(std::string(SHADERS_DIR),"segment_as_tube");
		um_assert(no_gl_error());
		nsegments = nedges;
		plop(nedges);
		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER,6*nsegments* sizeof(float),pts,GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*) (3*sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}




	void render(){

		um_assert(no_gl_error());
		glEnable(GL_DEPTH_TEST);
		shaderProgram=God::shaders["segment_as_tube"];
		glUseProgram(shaderProgram);
		glDisable(GL_LINE_SMOOTH);
		GLfloat range[2];
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE,range);
		//printf("Line width range: %f .. %f\n",range[0],range[1]);		
		double radius =.55;
		auto [w,h] = God::context.screen_size();
		float lineWidth =
			2.0f
			* radius
			* God::camera.impl->projection_matrix()[1][1]
			* float(h);

		//glLineWidth(2.*lineWidth);
		glLineWidth(20);
		//glLineWidth(20);



		declare_projection_matrix();
		declare_inv_projection_matrix();
		declare_view_matrix();
		declare_model_matrix();
		declare_light_direction();
		declare_viewport();


		//float pointRadius = 2.*double(radius_in_pixel)/(God::camera.impl->projection_matrix(w,h)[1][1]*double(h));
		//glUniform1f(glGetUniformLocation(shaderProgram,"R"),pointRadius);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);

		glBindVertexArray(vao);
		glDrawArrays(GL_LINES,0,GLsizei(nsegments));
		glBindVertexArray(0);

		um_assert(no_gl_error());
	}

};


struct RenderTubes: public RenderLayer{

	std::string mm_name;
	std::string polyline_name;


	SegmentRenderer segment_renderer;


	void generate_gui(std::string name){

		ImGui::ColorEdit3(("MyColor##"+name).c_str(),(float*)&segment_renderer.color,ImGuiColorEditFlags_None);
	}

	bool handle(Event event){
		if(!God::xcf.contains(mm_name)) return false;
		if(!God::xcf[mm_name].polylines.contains(polyline_name)) return false;
		return true;
	}

	void init(std::string mm,std::string polyline){
		polyline_name = polyline;
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		um_assert(God::xcf[mm_name].polylines.contains(polyline_name));
		PolyLine&  pl= God::xcf[mm_name].polylines[polyline_name].mesh;
		
		//God::xcf[mm_name].save_to_path("C:/NICOTMP/out");
		BBox3 box;
		for(auto v:pl.iter_vertices()) box.add(v.pos());

		std::vector<float> vertices(12*pl.nedges(),0);
		for(auto e:pl.iter_edges()){
			FOR(d,3) vertices[12*e+d] = 2.*(e.from().pos()-box.center())[d]/box.size().norm();
			FOR(d,3) vertices[12*e+3+d] = 2.*(e.to().pos()-box.center())[d]/box.size().norm();
			FOR(d,3) vertices[12*e+6+d] = 2.*(e.from().pos()-box.center())[d]/box.size().norm();
			FOR(d,3) vertices[12*e+9+d] = 2.*(e.to().pos()-box.center())[d]/box.size().norm();
		}
		segment_renderer.init(vertices.data(),2*pl.nedges());
	}


	void render(){
		segment_renderer.render();
	}
};



#include "ssao.h"









// -------------------------------------------------------------------------------
//                                    Modes to define the behavior of a specific application
// -------------------------------------------------------------------------------
// ==> not satisfying  ATM

namespace InteractionMode{
	struct AbstractMode{
		virtual void define_gui()=0;
		virtual void activate()=0;
		virtual void disactivate()=0;
	};

	struct LoadScreen: public AbstractMode{
		LoadScreen(){}
		virtual void define_gui();
		void activate(){}
		void disactivate(){}
	};

	struct MoveVertex: public AbstractMode{
		MoveVertex(){

		}
		void activate(){
		}
		void disactivate(){
		}
		void define_gui(){}

	};


	struct HexEdit: public AbstractMode{
		HexEdit(){
			mode=&load_screen;
			God::root_mode = this;
		}

		virtual void define_gui(){ mode->define_gui(); }
		void activate(){}
		void disactivate(){}

		void set_mode(AbstractMode& new_mode){
			mode->disactivate();
			mode = &new_mode;
			mode->activate();
		}

		AbstractMode* mode;

		LoadScreen load_screen;
		MoveVertex move_vertex;
	};



	// ------------------------------------------------------------
	// SOME METHODS USING GOD CANNOT BE DEFINED IN THEIR CLASS BEFORE 
	// SPLITTING THE PROJECT INTO MORE THAN ONE FILE
	// ------------------------------------------------------------

	void LoadScreen::define_gui(){
		ImGui::Begin("Load XCF",nullptr,ImGuiWindowFlags_AlwaysAutoResize);
		{
			ImGui::Text("Load XCF");
			ImGui::Separator();
			static char str0[128] = "Hello, world!";
			ImGui::InputText("input text",str0,128);


			//if(ImGui::Button("Create MultiMesh",ImVec2(180,40)))
			{
				God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B0.step.mesh",true);
				

				God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B1.step.mesh",true);
				HexEdit* root  =static_cast<HexEdit*>(God::root_mode);
				root->set_mode(root->move_vertex);

				God::layers.emplace_back<RenderLambertTriangles>("Lambert").init("B1.step","triangles");
				God::layers.emplace_back<RenderLambertTriangles>("Lambert2").init("B0.step","triangles");
				God::layers.emplace_back<RenderSpheres>("RenderSpheres").init("B0.step","triangles");
    						God::layers.emplace_back<SSAO>("SSAO").init();
				God::layers.emplace_back<RenderTubes>("RenderTubes").init("B0.step","polylines");

			}
		}
		ImGui::End();
	}
};












int main(){
	God::context.init();
	InteractionMode::HexEdit look;
	InteractionMode::AbstractMode* root_mode=&look;

	God::panels.emplace_back<XCFViewer>("xcf_window");
	God::panels.emplace_back<LayerViewer>("layer_window");
	while(God::context.window_is_active()){
		glfwPollEvents();
		God::context.begin_frame();
		God::layers.render();
		God::panels.show_gui();
		root_mode->define_gui();
		if (!ImGui::GetIO().WantCaptureMouse)
			God::mouse.update();
		if(!ImGui::GetIO().WantCaptureKeyboard || !ImGui::GetIO().WantCaptureMouse)
			God::keys.update();

		God::events.dispatch();
		
		God::context.end_frame();
	}
	return EXIT_SUCCESS;
}
