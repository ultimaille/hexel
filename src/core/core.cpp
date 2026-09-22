#define _USE_MATH_DEFINES
#include "core.h"
#include <cmath>

namespace God {
    XCF xcf;
    LayerManager layers;
    ShaderManager shaders;
    InteractionMode::AbstractMode* root_mode;
    MouseState mouse;
    KeyboardState keys;
    WindowContext context;
    Camera camera;
    std::vector<Event> events;
    PanelManager pan_manager;
}


// ------------------------------------------------------------
// GLFW callbacks
// ------------------------------------------------------------

void framebuffer_size_callback(GLFWwindow* window,int width,int height){
	glViewport(0,0,width,height);
}

void mouse_button_callback(GLFWwindow* window,int button,int action,int mods){
	ImGui_ImplGlfw_MouseButtonCallback(window,button,action,mods);
	if(button <0 || button>2) { Log::error("Do not manage mouses with more than 3 buttons"); return; }
	God::mouse.mouseDragging[button] = (action == GLFW_PRESS);
	if (God::mouse.mouseDragging[button])
		God::events.push_back({Event::MOUSE_PRESSED,""});
	else 
		God::events.push_back({Event::MOUSE_RELEASED,""});
}

void cursor_position_callback(GLFWwindow* window,double mouseX,double mouseY){
	ImGui_ImplGlfw_CursorPosCallback(window,mouseX,mouseY);
	God::mouse.lastx=mouseX; std::swap(God::mouse.lastx,God::mouse.x);
	God::mouse.lasty=mouseY; std::swap(God::mouse.lasty,God::mouse.y);
}

void scroll_callback(GLFWwindow* window,double xOffset,double yOffset){
	ImGui_ImplGlfw_ScrollCallback(window,xOffset,yOffset);
	God::mouse.set_wheel_event(yOffset);
}


// ------------------------------------------------------------
//  drawback of everything a a .h file :(
// ------------------------------------------------------------



void KeyboardState::update(){
	for(int k=512; k<ImGuiKey_Oem102; k++){
		bool nv =ImGui::IsKeyDown(ImGuiKey(k));
		if(nv!=data[k]) {
			if(nv)
				God::events.push_back({Event::KEY_PRESSED,""});
			else
				God::events.push_back({Event::KEY_RELEASED,""});
			//plop(k); //====> run callbacks
		}
		data[k] =nv;
	}
}
