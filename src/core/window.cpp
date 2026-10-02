#include "core.h"

// ------------------------------------------------------------
// GLFW callbacks
// ------------------------------------------------------------

using namespace events;

void framebuffer_size_callback(GLFWwindow* window,int width,int height){
    God::context.resize_framebuffer(width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window,button,action,mods);
    um_assert(button >= 0 && button < MouseState::ButtonCount);
    God::mouse.current.buttons[button] = (action == GLFW_PRESS);
    ObjectId(MOUSE).broadcast( UPDATED );
}

void cursor_position_callback(GLFWwindow* window, double x, double y) {
    ImGui_ImplGlfw_CursorPosCallback(window, x, y);
    ObjectId(MOUSE).broadcast( UPDATED );
    God::mouse.current.x = x;
    God::mouse.current.y = y;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    God::mouse.wheel_speed = yoffset;
    ObjectId(MOUSE).broadcast( UPDATED );
}

void KeyboardState::update() {
    for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
        bool down = glfwGetKey(God::context.window, k) == GLFW_PRESS;
        if (down != keys[k])
            ObjectId(KEYBOARD).broadcast( UPDATED );
        keys[k] = down;
    }
}

