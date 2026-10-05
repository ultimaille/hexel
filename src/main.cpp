#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <ultimaille/all.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdlib>
#include <iostream>

#include <string>

#include <ultimaille/all.h>
#include "core/core.h"

#include "panels/property_explorer.h"
#include "panels/xcf_explorer.h"
#include "panels/layer_explorer.h"

#include "layers/basic_layers.h"
#include "layers/ssao.h"

// -------------------------------------------------------------------------------
//                                    Modes to define the behavior of a specific application
// -------------------------------------------------------------------------------
// ==> not satisfying  ATM

namespace InteractionMode{
    struct AbstractMode{
        virtual void define_gui() = 0;
    };

    struct HexEdit : public AbstractMode{
        HexEdit(){
            God::root_mode = this;
            God::layers.add<SSAO>("SSAO").init();
        }

        std::string to_string(events::ObjectType path) {
			switch (path) {
			case events::ObjectType::MULTIMESH:     return "multimesh";
			case events::ObjectType::POINTSET:      return "pointset";
			case events::ObjectType::TRIANGLES:     return "triangles";
			case events::ObjectType::POLYLINES:     return "polylines";
			};

            return "???";
        }
        std::string to_string(ObjectId obj) {
            std::string res = to_string(obj.path)+": ";
            for (auto& s : obj.names) res += "->"+s;
            return  res;
        }
        
        void command_gui() {
            static ObjectId arg0;
            bool press = ImGui::Button(label(to_string(arg0), "command_gui"));
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECTID")) {
                    arg0 = **(const ObjectId**)payload->Data;
                    //delete static_cast<ObjectId*>(*(ObjectId**)payload->Data);
                    Log::add(std::to_string(arg0.path));
                    for(auto s: arg0.names) Log::add(s);
                    Log::add(to_string(arg0));
                } 
                ImGui::EndDragDropTarget();
            }
            
            if (press) {
                Log::add("need to switch to select arg0");
            }


            if (ImGui::BeginMenu("Run...")) {
                ImGui::MenuItem("create");
                if (ImGui::MenuItem("New")) {}

                if (ImGui::BeginMenu("sous menu")) {
                    ImGui::MenuItem("fish_hat.h");
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
        }

        void define_gui() {
            ImGui::Begin("ModeWindow", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Window used to launch debug tests");
            if (ImGui::Button("LoadSomething")) {
                // Horrible way to acces a function: but it's good to check that it works
                XCFExplorer& pan = dynamic_cast<XCFExplorer&> (static_cast<Panel&>(ObjectId(PANEL, "xcf_window")));
                pan.load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "mmB0");
            }

            if (ImGui::Button("CurrentTest")) {
                Log::add("Starting new test");
            }
            ImGui::Separator();
            command_gui();

            ImGui::End();
        }
    };
};






int main(int argc, const char* argv[]) {
    God::context.init();
    InteractionMode::HexEdit look;
    God::panels.add<XCFExplorer>("xcf_window");
    God::panels.add<LayerExplorer>("layer_window");
    God::panels.add<PropertyExplorer>("property_window");


    if (argc > 1) {
        std::string path = argv[1];
    }

    while(God::context.window_is_active()){
        God::mouse.wheel_speed = 0;
        God::mouse.previous = God::mouse.current;
        glfwPollEvents();
        if(!ImGui::GetIO().WantCaptureKeyboard || !ImGui::GetIO().WantCaptureMouse)
            God::keys.update();
        God::events.dispatch();

        God::context.begin_frame();
        God::layers.render();
        God::panels.show_gui();
        God::root_mode->define_gui();
        God::context.end_frame();
    }
    God::layers.destroy();
    God::context.destroy();
    return EXIT_SUCCESS;
}
