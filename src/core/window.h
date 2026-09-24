#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


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

	void update();


	bool mouseDragging[3] ;
	double lastx,lasty,x,y;
	double wheel_event_speed;
};

struct KeyboardState{
	
	bool pressed(int key /* GLFW_KEY_? */){ return data[key]; }

	void update();

	std::array<bool,2048> data;

};
