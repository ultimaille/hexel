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

#include "core/objectid.h"

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

    while (God::context.window_is_active()){
        God::mouse.wheel_speed = 0;
        God::mouse.previous = God::mouse.current;
        glfwPollEvents();
        if (!ImGui::GetIO().WantCaptureKeyboard || !ImGui::GetIO().WantCaptureMouse)
            God::keys.update();

    
        God::context.begin_frame();
        
        for (auto &group : God::layers.groups) {
            God::context.render_target.bind(group.id);
            God::layers.render(group.id);
        }
        
        God::events.dispatch();

        God::panels.show_gui();
        God::modes.define_gui();
        God::context.end_frame();
    }
    God::layers.destroy();
    God::context.destroy();
    return EXIT_SUCCESS;
}
