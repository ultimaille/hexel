#include "misc/purgatory.h"
#include "layers/ssao.h"





struct MouseReactPickerTest : public MouseReact {
    MouseReactPickerTest() {  }

    void init(ObjectId p_attr_id) {
        attr_id = p_attr_id;
        flag.bind(
            attr_id.names[4],
            God::xcf[attr_id.names[0]].hexahedra[attr_id.names[1]].attributes,
            God::xcf[attr_id.names[0]].hexahedra[attr_id.names[1]].mesh
        );
    }
    ObjectId attr_id;
    CellFacetAttribute<int> flag;


    void on_wheel(double v) { Log::add("Wheel " + std::to_string(v)); }
    void on_click(int button, vec2 p) {
        Log::add("click " + std::to_string(button));
        Picker picker;
        auto pr = picker.at(p);

        ObjectId mesh(ObjectType::HEXAHEDRA, { attr_id.names[0],attr_id.names[1] });
        FOR(l , God::layers.size()) {
            if (God::layers[l].id != pr.layer_id)       continue;
            if (God::layers[l].mesh != mesh)            continue;
            if (God::layers[l].primitive_type() != 2)   continue;
            if (button == 0) flag[pr.primitive_id]++;
            if (button == 1) flag[pr.primitive_id]--;
            mesh.broadcast(EventType::UPDATED);
        }
        Log::add("layer id: " + std::to_string(pr.layer_id));
        Log::add("primitive id: " + std::to_string(pr.primitive_id));
        Log::add("object id: " + to_string(pr.object_id));
    }
    void on_press(int button, vec2 p)           { Log::add("press " + std::to_string(button)); }
    void on_release(int button, vec2 p)         { Log::add("release " + std::to_string(button)); }
    void on_drag(int button, vec2 a, vec2 b)    { Log::add("drag " + std::to_string(button)); }
};


struct HexEdit : public Mode{

    MouseReactTrackBallCamera camera_react;
    MouseReactPickerTest pick_react;

    HexEdit(){}
    void handle(Event event) { 
        
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
            send_instruction_to_react(event,camera_react);
        else {
            send_instruction_to_react(event, pick_react);
        }
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

            {// some attributes for rendering tests
                CellFacetAttribute<float> cfa("cfa", attr, hex);
                for (auto f : hex.iter_facets()) cfa[f] = f;
                CellCornerAttribute<float> cca("cca", attr, hex);
                for (auto c : hex.iter_corners()) cca[c] = c;
                CellAttribute<bool> visible("visible", attr, hex);
                for (auto c : hex.iter_cells()) visible[c] = c % 2;
            }


            {// a flag attibute for painting test
                
                //create and init attribute
                CellFacetAttribute<int> flag("flag", attr, hex);
                for (auto f : hex.iter_facets()) flag[f] = 0;
                ObjectId flag_id(ObjectType::HEXAHEDRA_ATTR, { "hexski", "","int","facets","flag" });
                
                // bind the attribute to the paint mouse action
                pick_react.init(flag_id);

                // set rendering
                HexahedraLayer& hex_layer = dynamic_cast<HexahedraLayer&>(God::layers["hexskiHex"]);
                hex_layer.attr_id = flag_id;
                hex_layer.reset();

            }
            
            //load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1.geogram");
            //God::xcf["B1"].save_to_path(std::string(TEST_INPUT_DIR) + "B1", false);
            //load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1");
            
            God::layers.add<SSAO>("SSAO").init();
        }

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        command_gui();


        ImGui::End();
    }
};
