#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "render_target.h"

void framebuffer_size_callback(GLFWwindow* window,int width,int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

struct WindowContext {
    GLFWwindow* window = nullptr;
    RenderTarget render_target = {};

    void init(int w = 1000, int h = 1000);

    ~WindowContext();

    void init_mouse_call_backs();

    void init_glfw(int& w, int& h);

    void init_glad();

    void init_imgui();

    void bind_render_target();

    void bind_default_framebuffer();

    void begin_frame(bool offscreen = false);

    void begin_scissor(vec4 rect);

    void end_scissor();

    void present_render_target();

    void resize_framebuffer(int width, int height);

    void end_frame(bool offscreen = false);

    std::pair<int, int> screen_size();

    bool window_is_active() const;

    void destroy();
};

// -------------------------------------------------------------------------------
//                                    Mouse + Keyboard States
// -------------------------------------------------------------------------------

struct MouseState {
    static constexpr int ButtonCount = GLFW_MOUSE_BUTTON_LAST + 1;
    struct State {
        double x = 0, y = 0;
        bool buttons[ButtonCount] = {};
    };

    double wheel_speed = 0;
    State current = {}, previous = {};

    bool clicked(int button) const {
        return current.buttons[button] && !previous.buttons[button];
    }

    bool released(int button) const {
        return !current.buttons[button] && previous.buttons[button];
    }

    bool down(int button) const {
        return current.buttons[button];
    }
};

struct KeyboardState {
    static constexpr int ButtonCount = GLFW_KEY_LAST + 1;
    bool pressed(int k /* GLFW_KEY_? */) { return keys[k]; }
    void update();
    std::array<bool, ButtonCount> keys = {};
};

