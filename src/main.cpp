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
			God::events.push_back({Event::MM_REMOVED,name});
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

	bool resync_with_data(){
		if(!God::xcf.contains(mm_name)) return false;
		if(!God::xcf[mm_name].triangles.contains(triangle_name)) return false;
//	if (!God::xcf[mm_name].triangles[triangle_name].modified) return true; // TODO: obsolete, à refaire avec la newsletter
		// Log::add("need to update vbo");
		return true;
	}

	void init(std::string mm,std::string triangle){
		triangle_name = triangle;
		mm_name = mm;
		um_assert(God::xcf.contains(mm_name));
		um_assert(God::xcf[mm_name].triangles.contains(triangle_name));
		Triangles&  tri = God::xcf[mm_name].triangles[triangle_name].mesh;

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
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_TRUE,God::camera.projection());

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_TRUE,God::camera.view());
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_TRUE,model);

		glUniform3f(glGetUniformLocation(shaderProgram,"lightPos"),1.0f,1.0f,0.0f);
		glUniform3f(glGetUniformLocation(shaderProgram,"lightColor"),0.9f,0.5f,0.5f);
		glUniform3f(glGetUniformLocation(shaderProgram,"objectColor"),color[0],color[1],color[2]);


		glDrawElements(GL_TRIANGLES,ncorners,GL_UNSIGNED_INT,nullptr);
		glBindVertexArray(0);
	}
};



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


			if(ImGui::Button("Create MultiMesh",ImVec2(180,40))){
				God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B0.step.mesh");
				God::xcf["B0.step"].triangles["B0.step"].mesh.connect();
				God::layers.emplace_back<RenderLambertTriangles>("Lambert").init("B0.step","triangles");

				God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B1.step.mesh");
				God::xcf["B1.step"].triangles["B1.step"].mesh.connect();
				God::layers.emplace_back<RenderLambertTriangles>("Lambert2").init("B1.step","triangles");
				HexEdit* root  =static_cast<HexEdit*>(God::root_mode);
				root->set_mode(root->move_vertex);
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
		God::camera.update();
		God::layers.sync();
		God::context.begin_frame();
		God::layers.render();
		God::panels.show_gui();
		root_mode->define_gui();
		if (!ImGui::GetIO().WantCaptureMouse)
			God::mouse.update();
		if(!ImGui::GetIO().WantCaptureKeyboard || !ImGui::GetIO().WantCaptureMouse)
			God::keys.update();
		God::context.end_frame();
	}
	return EXIT_SUCCESS;
}
