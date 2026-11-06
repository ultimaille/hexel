#include <modes/submode_basic_edit.h>



    BasicEditMode::BasicEditMode(HexModelerMode& mode) : HexModelerMode::SubMode(mode){
    }


    void BasicEditMode::handle(Event event) {
        if (event.who.path != ObjectType::MOUSE) return;
        // forward mouse events to mouse reacts 
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
            mode.handle_camera_and_layerselect(event);
        else {
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

