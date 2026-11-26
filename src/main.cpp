#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <ultimaille/all.h>
#include <imgui.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include <core/all.h>

#include "panels/main_menu.h"
#include "panels/xcf_explorer.h"
#include "panels/property_explorer.h"
#include "panels/layer_explorer.h"

#include "misc/purgatory.h"

#include "layers/basic_layers.h"
#include "layers/ssao.h"

#ifdef HAS_CONFIG
    #include "myconfig.h"
#endif

int main(int argc, const char* argv[]) {
    God::context.init();

    God::modes.add<DefaultMode>("default");
    #ifdef HAS_CONFIG
    Config::init_modes();
    #endif

    God::panels.add<MainMenu>("main_menu");
    God::panels.add<XCFExplorer>("xcf_window");
    God::panels.add<LayerExplorer>("layer_window");
    God::panels.add<PropertyExplorer>("property_window");


    if (argc > 1) {
        std::string path = argv[1];
    }

    constexpr int settle_frames = 3;
    int redraw_frames = settle_frames;

    while (God::context.window_is_active()) {
        God::mouse.wheel_speed = 0;
        God::mouse.previous = God::mouse.current;
        if (redraw_frames > 0) {
            glfwPollEvents();
        } else {
            glfwWaitEventsTimeout(.5);
            redraw_frames = settle_frames;
        }
        if (!ImGui::GetIO().WantCaptureKeyboard || !ImGui::GetIO().WantCaptureMouse)
            God::keys.update();
        God::modes.update();
        God::context.begin_frame();
        God::context.render_target.render_groups();
        God::events.dispatch();

        God::panels.show_gui();
        God::modes.define_gui();
        God::context.end_frame();

        if (!God::events.queue.empty())
            redraw_frames = settle_frames;
        --redraw_frames;
    }
    God::layers.destroy();
    God::context.destroy();
    return EXIT_SUCCESS;
}
