#include "misc/purgatory.h"
#include "layers/ssao.h"





struct MouseReactPickerTest : public MouseReact {
    MouseReactPickerTest(int filter = 0) { MouseReact::active_sub_modes = {filter}; }

    void on_wheel(double v) { Log::add("Wheel " + std::to_string(v)); }
    void on_click(int button, vec2 p) {
        Log::add("click " + std::to_string(button));
        Picker picker;
        return;
        auto pr = picker.at(p);
        Log::add("layer id: " + std::to_string(pr.layer_id));
        Log::add("primitive id: " + std::to_string(pr.primitive_id));
        Log::add("object id: " + to_string(pr.object_id));
    }
    void on_press(int button, vec2 p)           { Log::add("press " + std::to_string(button)); }
    void on_release(int button, vec2 p)         { Log::add("release " + std::to_string(button)); }
    void on_drag(int button, vec2 a, vec2 b)    { Log::add("drag " + std::to_string(button)); }
};


struct HexEdit : public Mode{

    HexEdit(){
        God::layers.add<SSAO>("SSAO").init();
        mouse_react.emplace_back<MouseReactTrackBallCamera>("camera").active_sub_modes = { 1 };
        mouse_react.emplace_back < MouseReactPickerTest>("picktest");
    }

    void handle(Event event) { 
        sub_mode = 0;
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL)) sub_mode += 1;
        if (God::keys.pressed(GLFW_KEY_LEFT_SHIFT)) sub_mode += 2;
        handle_mouse(event); 
    }


    void command_gui() {
        static ObjectId arg0;
        ImGui::TextColored(ImVec4(1, .8, .8, 1.), "Target"); ImGui::SameLine();
        bool press = ImGui::Button(label(to_string(arg0), "command_gui"));
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECTID")) {
                arg0 = **(const ObjectId**)payload->Data;
                Log::add(std::to_string(arg0.path));
                for (auto s : arg0.names) Log::add(s);
                Log::add(to_string(arg0));
            }
            ImGui::EndDragDropTarget();
        }

        if (press) Log::add("need to switch to select arg0");
        if (ImGui::BeginMenu("Run...")) {
            ImGui::MenuItem("create");
            if (ImGui::MenuItem("New")) {}
            if (ImGui::BeginMenu("sous menu")) {
                ImGui::MenuItem("glo");
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
    }




    void define_gui() {
        ImGui::Begin("Mode", nullptr);
        ImGui::Text("Window used to launch debug tests");
        if (ImGui::Button("LoadSomething")) {
            load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "hexski.geogram");
            
            auto& attr = God::xcf["hexski"].hexahedra[""].attributes;
            auto& hex = God::xcf["hexski"].hexahedra[""].mesh;

            CellFacetAttribute<float> cfa("cfa", attr, hex);
            for (auto f : hex.iter_facets()) cfa[f] = f;
            CellCornerAttribute<float> cca("cca", attr, hex);
            for (auto c : hex.iter_corners()) cca[c] = c;
            CellAttribute<bool> visible("visible", attr, hex);
            for (auto c : hex.iter_cells()) visible[c] = c%2;
            God::layers["hexskiHex"].reset();

            //load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1.geogram");
            //God::xcf["B1"].save_to_path(std::string(TEST_INPUT_DIR) + "B1", false);
            //load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1");
        }

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        command_gui();


        ImGui::End();
    }
};
