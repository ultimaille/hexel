#include "window.h"

#include "basic.h"
#include "core.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"
#include "mode.h"

// ------------------------------------------------------------
// GLFW callbacks
// ------------------------------------------------------------

using namespace events;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    God::context.resize_framebuffer(width, height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    um_assert(button >= 0 && button < MouseState::ButtonCount);
    God::mouse.current.buttons[button] = (action == GLFW_PRESS);
    ObjectId(MOUSE).broadcast(UPDATED);
}

void cursor_position_callback(GLFWwindow* window, double x, double y) {
    ImGui_ImplGlfw_CursorPosCallback(window, x, y);
    ObjectId(MOUSE).broadcast(UPDATED);
    God::mouse.current.x = x;
    God::mouse.current.y = y;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
    God::mouse.wheel_speed = yoffset;
    ObjectId(MOUSE).broadcast(UPDATED);
}

void KeyboardState::update() {
    for (int k = 0; k <= GLFW_KEY_LAST; ++k) {
        bool down = glfwGetKey(God::context.window, k) == GLFW_PRESS;
        if (down != keys[k])
            ObjectId(KEYBOARD).broadcast(UPDATED);
        keys[k] = down;
    }
}





void WindowContext::init(int w, int h) {
    init_glfw(w, h);
    init_glad();

    // The requested GLFW window size and the actual OpenGL framebuffer size can differ (for example, on HiDPI displays)
    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

    if (framebuffer_width > 0 && framebuffer_height > 0) {
        render_target.init(framebuffer_width, framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
    }

    init_imgui();
    init_mouse_call_backs();
}

WindowContext::~WindowContext() {
    //      TODO: add a correct shutdown();
}

void WindowContext::init_mouse_call_backs() {
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void WindowContext::init_glfw(int& w, int& h) {
    um_assert(glfwInit());
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    GLFWmonitor* monitor = (count > 1) ? monitors[1] : monitors[0];
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    int monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);

    w = mode->width;
    h = mode->height - 30;
    window = glfwCreateWindow(w, h, "Hexel", nullptr, nullptr);
    glfwSetWindowPos(window, monitorX, monitorY + 30);

    um_assert(window != nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

void WindowContext::init_glad() {
    int version = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
    um_assert(version != 0);
    Log::add(std::string("OpenGL version: ") + std::string((char*)glGetString(GL_VERSION)));
}

void WindowContext::init_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    um_assert(ImGui_ImplGlfw_InitForOpenGL(window, true));
    um_assert(ImGui_ImplOpenGL3_Init("#version 330"));
}

void WindowContext::bind_render_target() {
    render_target.bind();
}

void WindowContext::bind_default_framebuffer() {
    RenderTarget::bind_default(render_target.width, render_target.height);
}

void WindowContext::begin_frame(bool offscreen) {
    if (!render_target.valid()) {
        Log::error("RenderTarget is invalid");
        return;
    }
    render_target.clear(0.05f, 0.05f, 0.08f, 1.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    if (!offscreen) ImGui::NewFrame();
}

void WindowContext::begin_scissor(vec4 rect) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(rect[0], rect[1], rect[2], rect[3]);
}

void WindowContext::end_scissor() {
    glDisable(GL_SCISSOR_TEST);
}

void WindowContext::present_render_target() {
    if (!render_target.valid()) {
        Log::error("RenderTarget is invalid");
        return;
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, render_target.framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, render_target.width, render_target.height, 0, 0, render_target.width, render_target.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    bind_default_framebuffer();
}

void WindowContext::resize_framebuffer(int width, int height) {
    render_target.resize(width, height);
    glViewport(0, 0, width, height);
}

void WindowContext::end_frame(bool offscreen) {
    if (!offscreen) {
        present_render_target();
        ImGui::Render();
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}

std::pair<int, int> WindowContext::screen_size() {
    return { render_target.width, render_target.height };
}

bool WindowContext::window_is_active() const {
    return window != nullptr && !glfwWindowShouldClose(window);
}

void WindowContext::destroy() {
    render_target.destroy();
}
