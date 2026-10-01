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

    void init(int w = 1000,int h = 1000) {
        init_glfw(w, h);
        init_glad();

        // The requested GLFW window size and the actual OpenGL framebuffer size can differ (for example, on HiDPI displays)
        int framebuffer_width  = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

        if (framebuffer_width > 0 && framebuffer_height > 0) {
            render_target.init(framebuffer_width, framebuffer_height);
            glViewport(0, 0, framebuffer_width, framebuffer_height);
        }

        init_imgui();
        init_mouse_call_backs();
    }

    ~WindowContext() {
//      TODO: add a correct shutdown();
    }

    void init_mouse_call_backs() {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);
    }

    void init_glfw(int& w,int& h) {
        um_assert(glfwInit());
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);

        int count;
        GLFWmonitor** monitors = glfwGetMonitors(&count);
        GLFWmonitor* monitor = (count>0)? monitors[1] : monitors[0] ;
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        int monitorX, monitorY;
        glfwGetMonitorPos(monitor, &monitorX, &monitorY);

        w = mode->width;
        h = mode->height-30;
        window = glfwCreateWindow(w,h, "Hexel", nullptr, nullptr);
        // Positionnement au centre de l'écran choisi
        glfwSetWindowPos(window,monitorX ,monitorY +30);

        um_assert(window != nullptr);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
    }

    void init_glad() {
        int version = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
        um_assert(version != 0);
        Log::add(std::string("OpenGL version: ") + std::string((char*)glGetString(GL_VERSION)));
    }

    void init_imgui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
    	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        ImGui::StyleColorsDark();
        um_assert(ImGui_ImplGlfw_InitForOpenGL(window, true));
        um_assert(ImGui_ImplOpenGL3_Init("#version 330"));
    }

    void bind_render_target() {
        render_target.bind();
    }

    void bind_default_framebuffer() {
        RenderTarget::bind_default(render_target.width, render_target.height);
    }

    void begin_frame() {
        if (!render_target.valid()) {
            Log::error("RenderTarget is invalid");
            return;
        }
        render_target.clear(0.05f, 0.05f, 0.08f, 1.0f);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void present_render_target() {
        if (!render_target.valid()) {
            Log::error("RenderTarget is invalid");
            return;
        }

        glBindFramebuffer(GL_READ_FRAMEBUFFER, render_target.framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, render_target.width, render_target.height, 0, 0, render_target.width, render_target.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
        bind_default_framebuffer();
    }

    void resize_framebuffer(int width, int height) {
        render_target.resize(width, height);
        glViewport(0, 0, width, height);
    }

    void end_frame() {
        present_render_target();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    std::pair<int,int> screen_size() {
        return { render_target.width, render_target.height };
    }

    bool window_is_active() const {
        return window!=nullptr && !glfwWindowShouldClose(window);
    }

    void destroy() {
        render_target.destroy();
    }
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

