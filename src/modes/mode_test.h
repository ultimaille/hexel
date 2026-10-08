#include "misc/purgatory.h"
#include "layers/ssao.h"

std::string to_string(events::ObjectType path) {
    switch (path) {
    case events::ObjectType::KEYBOARD:          return "keyboard";
    case events::ObjectType::MOUSE:             return "mouse";
    case events::ObjectType::CAMERA:            return "camera";
    case events::ObjectType::PANEL:             return "panel";
    case events::ObjectType::LAYER:             return "layer";
    case events::ObjectType::MULTIMESH:         return "multimesh";
    case events::ObjectType::POINTSET:          return "pointset";
    case events::ObjectType::POLYLINES:         return "polylines";
    case events::ObjectType::TRIANGLES:         return "triangles";
    case events::ObjectType::QUADS:             return "quads";
    case events::ObjectType::POLYGONS:          return "polygons";
    case events::ObjectType::TETRAHEDRA:        return "tetrahedra";
    case events::ObjectType::HEXAHEDRA:         return "hexahedra";
    case events::ObjectType::WEDGES:            return "wedges";
    case events::ObjectType::PYRAMIDS:          return "pyramids";

    case events::ObjectType::POINTSET_ATTR:     return "pointset";
    case events::ObjectType::POLYLINES_ATTR:    return "polylines";
    case events::ObjectType::TRIANGLES_ATTR:    return "triangles";
    case events::ObjectType::QUADS_ATTR:        return "quads";
    case events::ObjectType::POLYGONS_ATTR:     return "polygons";
    case events::ObjectType::TETRAHEDRA_ATTR:   return "tetrahedra";
    case events::ObjectType::HEXAHEDRA_ATTR:    return "hexahedra";
    case events::ObjectType::WEDGES_ATTR:       return "wedges";
    case events::ObjectType::PYRAMIDS_ATTR:     return "pyramids";
    };
    return "XXX";
}
std::string to_string(ObjectId obj) {
    std::string res = to_string(obj.path);
    for (int i = 0; i < obj.names.size(); i++)
        if (!obj.names[i].empty())
            res += (i == 0 ? ": " : "->") + obj.names[i];
    return  res;
}



struct MouseReactPickerTest : public MouseReact {
    MouseReactPickerTest(int filter = 0) {
        MouseReact::filter = filter;
    }
    void on_wheel(double v) { Log::add("Wheel " + std::to_string(v)); }
    void on_click(int button, vec2 p) {
        Log::add("click " + std::to_string(button));

        Picker picker;
        return;
        auto [layer_id, primitive_id, object_id] = picker.at(p);
        Log::add("layer id: " + std::to_string(layer_id));
        Log::add("primitive id: " + std::to_string(primitive_id));
        Log::add("object id: " + to_string(object_id));
    }
    void on_press(int button, vec2 p)           { Log::add("press " + std::to_string(button)); }
    void on_release(int button, vec2 p)         { Log::add("release " + std::to_string(button)); }
    void on_drag(int button, vec2 a, vec2 b)    { Log::add("drag " + std::to_string(button)); }
};


struct HexEdit : public Mode{

    HexEdit(){
        God::layers.add<SSAO>("SSAO").init();
        mouse_react.emplace_back<MouseReactTrackBallCamera>("camera").filter = ctrl_pressed;
        mouse_react.emplace_back < MouseReactPickerTest>("picktest");
    }

    void handle(Event event) { handle_mouse(event); }


    void command_gui() {
        static ObjectId arg0;
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
            load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1.geogram");
            God::xcf["B1"].save_to_path(std::string(TEST_INPUT_DIR) + "B1", false);
            load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1");
        }

        if (ImGui::Button("CurrentTest")) {
            Log::add("Starting new test");
        }
        ImGui::Separator();
        command_gui();


        ImGui::End();
    }
};
