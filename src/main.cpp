#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <ultimaille/all.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdlib>
#include <iostream>

#include <string>

#include <core/all.h>
#include <modes/mode_test.h>

#include "panels/property_explorer.h"
#include "panels/xcf_explorer.h"
#include "panels/layer_explorer.h"

#include "misc/purgatory.h"

#include "layers/basic_layers.h"
#include "layers/ssao.h"





void main_menu_gui(){
    static std::string path; 

    bool load_mm = false;
    bool import_mesh = false;
    if (ImGui::BeginMainMenuBar()){
        if (ImGui::BeginMenu("Files")){

            if (ImGui::MenuItem("Load MultiMesh", NULL)) load_mm = true;
            if (ImGui::MenuItem("Import .geogram", NULL))import_mesh = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Modes")){
            ImGui::SeparatorText("Current mode");
            for (int i = 0; i < God::modes.size(); i++)
                if (ImGui::MenuItem(God::modes.ith_name(i).c_str(), NULL, God::modes.current_mode == i))
                    God::modes.current_mode = i;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (load_mm){ ImGui::OpenPopup("MMFilePopup"); load_mm = false; }
    if (FilePopup("MMFilePopup", std::string(TEST_INPUT_DIR), path, { ".mm" }))
        if (!path.empty()) load_mm_with_default_layers(path.substr(0, path.size() - 3));

    if (import_mesh){ ImGui::OpenPopup("ImportFilePopup"); import_mesh = false; }
    if (FilePopup("ImportFilePopup", std::string(TEST_INPUT_DIR), path, { ".geogram",".mesh",".meshb" }))
        if (!path.empty()) load_mm_with_default_layers(path.substr(0));
}



int main(int argc, const char* argv[]) {
    God::context.init();
    God::modes.add<DefaultMode>("default");
    God::modes.add<HexEdit>("hexedit");
    God::modes.current_mode = 1;

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
        God::events.dispatch();

        God::context.begin_frame();
        God::layers.render();

        main_menu_gui();
        God::panels.show_gui();
        God::modes.define_gui();
        God::context.end_frame();
    }
    God::layers.destroy();
    God::context.destroy();
    return EXIT_SUCCESS;
}
