#pragma once

#include <map>
#include <fstream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ultimaille/all.h>

#include "basic.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"


// -------------------------------------------------------------------------------
//                                    CONTEXT
// -------------------------------------------------------------------------------
// => init almost everything
// => start and finish frames
// => give access to screen size
// => GLFW callbacks will be remove by refactoring into more than one C++ file


void framebuffer_size_callback(GLFWwindow* window,int width,int height);
void mouse_button_callback(GLFWwindow* window,int button,int action,int mods);
void cursor_position_callback(GLFWwindow* window,double mouseX,double mouseY);
void scroll_callback(GLFWwindow* window,double xOffset,double yOffset);

struct WindowContext{

	void init(int w=1000,int h=1000){
		init_glfw(w,h);
		init_glad();
		init_imgui();
		init_mouse_call_backs();
	}
	~WindowContext(){
		// Not sure I want to quit nicely... especially after the main
		//ImGui_ImplOpenGL3_Shutdown();
		//ImGui_ImplGlfw_Shutdown();
		//ImGui::DestroyContext();
		//glfwDestroyWindow(window);
		//glfwTerminate();
	}

	void init_mouse_call_backs(){
		glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
		glfwSetMouseButtonCallback(window,mouse_button_callback);
		glfwSetCursorPosCallback(window,cursor_position_callback);
		glfwSetScrollCallback(window,scroll_callback);
	}
	void init_glfw(int w,int h){
		um_assert(glfwInit());
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
		glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DEPTH_BITS,24);
		window = glfwCreateWindow(1000,700,"Hexel",nullptr,nullptr);
		um_assert(window);

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);
	}
	void init_glad(){
		int version = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
		um_assert(version != 0);
		Log::add(std::string("OpenGL version: ") + std::string((char*)glGetString(GL_VERSION)));
	}
	void init_imgui(){
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |=ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		um_assert(ImGui_ImplGlfw_InitForOpenGL(window,true));
		um_assert(ImGui_ImplOpenGL3_Init("#version 330"));
	}


	void begin_frame(){
		int width = 0;
		int height = 0;

		//Opengl
		glfwGetFramebufferSize(window,&width,&height);
		glViewport(0,0,width,height);
		glClearColor(0.05f,0.05f,0.08f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
	}
	void end_frame(){
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	std::pair<int,int> screen_size(){
		int h; int w;
		glfwGetFramebufferSize(window,&w,&h);
		return {w,h};
	} ;
	bool window_is_active(){ return !glfwWindowShouldClose(window); }
	GLFWwindow* window;
};








// -------------------------------------------------------------------------------
//                                    ShaderManager
// -------------------------------------------------------------------------------
// => construct a dictonary of shaders 
// 


struct ShaderManager: public std::map<std::string,GLuint> {
	ShaderManager(){
	}
	GLuint compileShader(GLenum type,const char* source){
		GLuint shader = glCreateShader(type);
		glShaderSource(shader,1,&source,nullptr);
		glCompileShader(shader);
		GLint success = GL_FALSE;
		glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
		if(!success){
			char log[1024] = {};
			glGetShaderInfoLog(shader,sizeof(log),nullptr,log);
			Log::error("shader compilation failed",log);
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
	GLuint createShaderProgram(const char* vertexSource,const char* fragmentSource){

		GLuint vertexShader =compileShader(GL_VERTEX_SHADER,vertexSource);
		if(!vertexShader)return 0;

		GLuint fragmentShader =compileShader(GL_FRAGMENT_SHADER,fragmentSource);
		if(!fragmentShader){ glDeleteShader(vertexShader); return 0; }


		GLuint program = glCreateProgram();
		glAttachShader(program,vertexShader);
		glAttachShader(program,fragmentShader);
		glLinkProgram(program);

		GLint success = GL_FALSE;
		glGetProgramiv(program,GL_LINK_STATUS,&success);

		if(!success){
			char log[1024] = {};
			glGetProgramInfoLog(program,sizeof(log),nullptr,log);
			Log::error("shader linking failed",log);
			glDeleteProgram(program);
			program = 0;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return program;
	}
	std::string loadShader(const std::string& filename){
		std::ifstream file(filename);
		if(!file) Log::error("Cannot open shader: ",filename);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
	void add(std::string path,std::string shader_name){
		if(find(shader_name)!=end()) return;
		operator[](shader_name)=createShaderProgram(loadShader(path+shader_name+std::string(".vert")).c_str(),loadShader(path+shader_name+std::string(".frag")).c_str());
		um_assert(operator[](shader_name));
	}
};


// -------------------------------------------------------------------------------
//                                    RenderLayer + LayerManager
// -------------------------------------------------------------------------------
// => 
// 


struct RenderLayer{
    virtual ~RenderLayer() = default;
	virtual void render()		=0;
	virtual void generate_gui(std::string name) =0;
	virtual bool resync_with_data()=0;
	virtual void render_primitive_id()				{Log::add("To be implemented");}
	virtual void render_constant_color(int layerid)	{Log::add("To be implemented"); }
	bool visible;
};

struct LayerManager: public Registry<RenderLayer> {
	void render(){
		FOR(i,size()) operator[](i).render();
	}
	void sync(){
		FOR(i,size()) if (!operator[](i).resync_with_data()){
			std::swap(registry[i],registry.back());
			registry.pop_back();
		};
	}
	
	void produce_picking_image(int* data,int w,int h){ Log::add("To be implemented"); }

};





// -------------------------------------------------------------------------------
//                                    Mouse + Keyboard States
// -------------------------------------------------------------------------------

struct MouseState{
	MouseState(){
		FOR(i,3) mouseDragging[i] = false;
		x = 0.0; y = 0.0; lastx = 0.0; lasty = 0.0;
		wheel_event_speed=0;
	}
	void set_wheel_event(double v){
		wheel_event_speed=v;
	}
	double get_wheel_event(bool consume=true){
		double ret = wheel_event_speed;
		if(consume) wheel_event_speed=0;
		return ret;
	}


	bool mouseDragging[3] ;
	double lastx,lasty,x,y;
	double wheel_event_speed;
};

struct KeyboardState{
	// adressed by ImGuiKey
	bool pressed(ImGuiKey key){ return data[key]; }

	void update();

	std::array<bool,2048> data;

};

// -------------------------------------------------------------------------------
//                                    Panels to explore things (layers/xcf/etc.)
// -------------------------------------------------------------------------------
// => are more or less independant of the mode
struct Panel{
    virtual ~Panel() = default;
	virtual void generate_gui()=0;
};
struct PanelManager{
	Registry<Panel> panels;
	void show_gui(){
		for(auto& [name,obj] : panels.registry) obj->generate_gui();
	}
};


// -------------------------------------------------------------------------------
//                                    Events
// -------------------------------------------------------------------------------
// we need some way to have objects interactions. Messages, listeners and callback would not simplify the conception
struct Event{
	enum {
		MOUSE_MOVED,
		MOUSE_PRESSED,
		MOUSE_RELEASED,
		KEY_PRESSED,
		KEY_RELEASED,
		MM_REMOVED,
		TRIANGLES_REMOVED,
		TRIANGLES_UPDATED
	} even_type;

	std::string object_name;// a string that allows to find the element
};

// -------------------------------------------------------------------------------
//                                    God: you know... he knows
// -------------------------------------------------------------------------------

namespace InteractionMode{ struct AbstractMode; }

namespace God{
	// current interaction mode (controls everything: it is the entry point for different tools)
	extern InteractionMode::AbstractMode* root_mode;

	// datas actually manipulated by the modeler	
	extern XCF xcf;

	// input state
	extern MouseState mouse;
	extern KeyboardState keys;

	// event that occurred since last frame
	extern std::vector<Event> events;

	// the camera
	extern Camera camera;
	extern LayerManager layers;							// layers to be combined into the final rendering

	extern PanelManager pan_manager;

	// API dependant 
	extern WindowContext context;
	extern ShaderManager shaders;
};


