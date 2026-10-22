#pragma once
#include "misc/purgatory.h"
#include "layers/ssao.h"

#include "algo.h"

struct MouseReactPickerTest2 : public MouseReact {
    MouseReactPickerTest2() {  }

    void init(ObjectId p_attr_id, ObjectId p_mesh) {
        // attr_id = p_attr_id;
        // mesh_id = p_mesh;
        // flag.bind(
        //     attr_id.names[4],
        //     God::xcf[attr_id.names[0]].hexahedra[attr_id.names[1]].attributes,
        //     God::xcf[attr_id.names[0]].hexahedra[attr_id.names[1]].mesh
        // );
    }
    // ObjectId attr_id;
    // ObjectId mesh_id;
    
    void on_click(int button, vec2 p) {
        Log::add("click " + std::to_string(button));
        Picker picker;

        for (auto &d : picker.data) {
            auto result = d.at(p);
            std::cout << "coords: (" << result.x << ", " << result.y << ")" << std::endl;
            std::cout << "layer_id: " << result.layer_id << std::endl;
            std::cout << "primitive_id: " << result.raw_primitive_id << std::endl;
            std::cout << "depth: " << result.depth << std::endl;
            std::cout << "point: " << result.point() << std::endl;
        }
    }
};

struct BenjiMode : public Mode{

    MouseReactTrackBallCamera camera_react;
    MouseReactPickerTest2 pick_react;

    BenjiMode(){}

    void handle(Event event) { 
        
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
            send_instruction_to_react(event,camera_react);
        else {
            send_instruction_to_react(event, pick_react);
        }
    }


    void define_gui() {
        ImGui::Begin("Mode", nullptr);
        ImGui::Text("Window used to launch debug tests");
        if (ImGui::Button("LoadSomething")) {
            load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B0.step.mesh");
            load_mm_with_default_layers(std::string(TEST_INPUT_DIR) + "B1.geogram");
        }
        ImGui::End();
    }
};