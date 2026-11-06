#include <modes/submode_basic_edit.h>



MouseReactKillHex::MouseReactKillHex(BasicEditMode& mode) :mode(mode){}

void MouseReactKillHex::on_click(int button, vec2 p) {
    if (button != 1) return;
    Picker picker;
    for (auto& d : picker.data) {
        auto result = d.at(p);
        int picked_cell = result.filtered_primitive_id(mode.hex_id(), 1);
        if (picked_cell != -1){
            Hexahedra& hex = mode.hex_id();
            CellAttribute<int> block_id("block_id", mode.hex_id(), hex);
            int block2kill = block_id[picked_cell];
            std::vector<bool> to_kill(hex.ncells());
            for (auto c : hex.iter_cells()) to_kill[c] = block_id[c] == block2kill;
            hex.delete_cells(to_kill);
            hex.connect();
            mode.update_hex();
        }
    }
}




MouseReactCreateHex::MouseReactCreateHex(BasicEditMode& mode) :mode(mode){}

void MouseReactCreateHex::on_click(int button, vec2 p) {
    //if (button != 1) return;
    Picker picker;
    for (auto& d : picker.data) {
        auto result = d.at(p);

        std::cout << "coords: (" << result.x << ", " << result.y << ")" << std::endl;
        std::cout << "layer_id: " << result.layer_id << std::endl;
        std::cout << "primitive_id: " << result.raw_primitive_id << std::endl;
        std::cout << "depth: " << result.depth << std::endl;
        std::cout << "point: " << result.point() << std::endl;

        int picked_vert = result.filtered_primitive_id(mode.hex_id(), 0);
        if (picked_vert != -1){
            plop(picked_vert);
            Hexahedra& hex = mode.hex_id();
            mode.update_hex();
        }
    }
    for (auto& d : picker.data) {
        auto result = d.at(p);
        std::cout << "coords: (" << result.x << ", " << result.y << ")" << std::endl;
        std::cout << "layer_id: " << result.layer_id << std::endl;
        std::cout << "primitive_id: " << result.raw_primitive_id << std::endl;
        std::cout << "depth: " << result.depth << std::endl;
        std::cout << "point: " << result.point() << std::endl;


        int picked_triangle = result.filtered_primitive_id(mode.geom_tri_id(), 1);
        if (picked_triangle != -1){
            plop(picked_triangle);
            Hexahedra& hex = mode.hex_id();
            mode.update_hex();
        }
    }
}






    


    BasicEditMode::BasicEditMode(HexModelerMode& mode) : HexModelerMode::SubMode(mode), react_kill_hex(*this), react_create_hex(*this){
        
    }


    void BasicEditMode::handle(Event event) {
        if (event.who.path != ObjectType::MOUSE) return;
        // forward mouse events to mouse reacts 
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
            mode.handle_camera_and_layerselect(event);
        else {
            send_instruction_to_react(event, react_kill_hex);
            send_instruction_to_react(event, react_create_hex);
        }

        // consume mouse react "return values"
        Hexahedra& hex = hex_id();
        PolyLine& pl = pl_id();

    }

    void BasicEditMode::define_gui() {
            ImGui::Begin("HexModelerMode", nullptr);
            mode.header_gui_for_submodes();
            ImGui::End();
    }

