#include "core.h"

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

void cursor_position_callback(GLFWwindow* window, double mouseX, double mouseY){
    ImGui_ImplGlfw_CursorPosCallback(window,mouseX,mouseY);
    
    // God::mouse.lastx=mouseX; 
    // std::swap(God::mouse.lastx,God::mouse.x);
    
    // God::mouse.lasty=mouseY; 
    // std::swap(God::mouse.lasty,God::mouse.y);
}

void scroll_callback(GLFWwindow* window,double xOffset,double yOffset){
    ImGui_ImplGlfw_ScrollCallback(window,xOffset,yOffset);
    God::mouse.set_wheel_event(yOffset);
}

void KeyboardState::update(){
    // Poll key states for keys 0 to GLFW_KEY_LAST
    for(int k = 0; k <= GLFW_KEY_LAST; k++){
        int state = glfwGetKey(God::context.window, k);
        bool nv = (state == GLFW_PRESS);
        
        if(nv != data[k]) {
            if(nv)
                God::events.push_back({Event::KEY_PRESSED, ""});
            else
                God::events.push_back({Event::KEY_RELEASED, ""});
        }
        data[k] = nv;
    }
}

void MouseState::update() {
    double mx, my;
    glfwGetCursorPos(God::context.window, &mx, &my);
    lastx = x;
    lasty = y;
    x = mx;
    y = my;
    God::events.push_back({Event::MOUSE_MOVED, ""});
}