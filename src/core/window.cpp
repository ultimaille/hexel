#include "core.h"

// ------------------------------------------------------------
// GLFW callbacks
// ------------------------------------------------------------

void framebuffer_size_callback(GLFWwindow* window,int width,int height){
    God::context.resize_framebuffer(width, height);
}

void mouse_button_callback(GLFWwindow* window,int button,int action,int mods){
    ImGui_ImplGlfw_MouseButtonCallback(window,button,action,mods);
    if(button <0 || button>2) { Log::error("Do not manage mouses with more than 3 buttons"); return; }
    God::mouse.button_pressed[button] = (action == GLFW_PRESS);
    God::events.push_back({ { {EventIdChunk::mouse},{} },EventType::UPDATED });
}

void cursor_position_callback(GLFWwindow* window, double mouseX, double mouseY){
    ImGui_ImplGlfw_CursorPosCallback(window,mouseX,mouseY);
}

void scroll_callback(GLFWwindow* window,double xOffset,double yOffset){
    
    ImGui_ImplGlfw_ScrollCallback(window,xOffset,yOffset);

    if (yOffset != 0) {
        God::mouse.current_state.wheel_speed = yOffset;
        God::events.push_back({ { {EventIdChunk::mouse},{} },EventType::UPDATED });
    }
}

void KeyboardState::update(){
    // Poll key states for keys 0 to GLFW_KEY_LAST
    for(int k = 0; k <= GLFW_KEY_LAST; k++){
        int state = glfwGetKey(God::context.window, k);
        bool nv = (state == GLFW_PRESS);
        
        if(nv != data[k]) 
            God::events.push_back({ { {EventIdChunk::key},{} },EventType::UPDATED });
        
        data[k] = nv;
    }
}

void MouseState::update() {
    double mx, my;
    glfwGetCursorPos(God::context.window, &mx, &my);
    if (current_state.x != mx || current_state.y != my)
        God::events.push_back({ { {EventIdChunk::mouse},{} },EventType::UPDATED });
    current_state.x = mx;
    current_state.y = my;
}
