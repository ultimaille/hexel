#include "misc/purgatory.h"
#include "layers/ssao.h"

#include "algo.h"




struct MouseReactPickerTest : public MouseReact {
    MouseReactPickerTest() {  }

    void init(ObjectId p_attr_id, ObjectId p_mesh) {
        attr_id = p_attr_id;
        mesh_id = p_mesh;
        flag.bind(
            attr_id.names[4],
            God::xcf[attr_id.names[0]].hexahedra[attr_id.names[1]].attributes,
            God::xcf[attr_id.names[0]].hexahedra[attr_id.names[1]].mesh
        );
    }
    ObjectId attr_id;
    ObjectId mesh_id;
    CellFacetAttribute<int> flag;


    void on_wheel(double v) { Log::add("Wheel " + std::to_string(v)); }
    void on_click(int button, vec2 p) {
        Log::add("click " + std::to_string(button));
        Picker picker;
        Picker::Result pr = picker.at(p);
        int prim = pr.filtered_primitive_id(mesh_id, 2);
        if (prim != -1){
            if (button == 0) flag[pr.raw_primitive_id]++;
            if (button == 1) flag[pr.raw_primitive_id]--;
            mesh_id.broadcast(EventType::UPDATED);
        }
    }
    void on_press(int button, vec2 p)           { Log::add("press " + std::to_string(button)); }
    void on_release(int button, vec2 p)         { Log::add("release " + std::to_string(button)); }
    void on_drag(int button, vec2 a, vec2 b)    { Log::add("drag " + std::to_string(button)); }
};


struct MouseReactLayerSelectionTest : public MouseReact {
    MouseReactLayerSelectionTest() {}

    void init(ObjectId p_mesh) {
        mesh_id = p_mesh;
    }
    ObjectId mesh_id;

    void on_wheel(double v) { Log::add("Wheel " + std::to_string(v)); }
    void on_click(int button, vec2 p) {
        Log::add("click " + std::to_string(button));
        if (button != 0) return;
        Picker picker;
        Picker::Result pr = picker.at(p);
        int prim = pr.filtered_primitive_id(mesh_id, 2);
        if (prim != -1){
            VolumeAttributes& attr = mesh_id;
            Hexahedra& hex = mesh_id;
            CellAttribute<bool> visible("visible", attr, hex);
            CellFacetAttribute<int> layer_id(hex);
            compute_hex_layer_id( hex, layer_id);
            visible.fill(false);
            for (auto f : hex.iter_facets()) if (layer_id[f] == layer_id[prim])
                visible[f.cell()] = true;
            mesh_id.broadcast(EventType::UPDATED);
        }
    }
    void on_press(int button, vec2 p)           { Log::add("press " + std::to_string(button)); }
    void on_release(int button, vec2 p)         { Log::add("release " + std::to_string(button)); }
    void on_drag(int button, vec2 a, vec2 b)    { Log::add("drag " + std::to_string(button)); }
};


struct HexEdit : public Mode{

    MouseReactTrackBallCamera camera_react;
    MouseReactPickerTest pick_react;
    MouseReactLayerSelectionTest layer_selection_react;

    HexEdit(){}
    void handle(Event event) { 
        
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL)){
            send_instruction_to_react(event, camera_react);
            send_instruction_to_react(event, layer_selection_react);
        }
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
                for (auto f : hex.iter_facets()) flag[f] = f%2;
                ObjectId hex_id(ObjectType::HEXAHEDRA, std::vector<std::string>{ "hexski", ""});
                ObjectId flag_id(ObjectType::HEXAHEDRA_ATTR, { "hexski", "","int","facets","flag" });

                // bind the attribute to the paint mouse action
                pick_react.init(flag_id,hex_id);
                layer_selection_react.init(hex_id);

                // set rendering
                HexahedraLayer& hex_layer = dynamic_cast<HexahedraLayer&>(God::layers["hexskiHex"]);
                hex_layer.attr_id = flag_id;
                FOR(i, 2) hex_layer.tri_renderer.data_range[i] = -4+8*i;
                hex_layer.reset();
            }
            
            //load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1.geogram");
            //God::xcf["B1"].save_to_path(std::string(TEST_INPUT_DIR) + "B1", false);
            //load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1");
            
            God::layers.add<SSAO>("SSAO", -1).init();
        }

        ImGui::Separator();
        ImGui::Separator();
        ImGui::Separator();
        command_gui();


        ImGui::End();
    }
};
