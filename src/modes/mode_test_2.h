#include "misc/purgatory.h"
#include "layers/ssao.h"

#include "algo.h"

struct BenjiMode : public Mode{

    MouseReactTrackBallCamera camera_react;
    MouseReactPickerTest pick_react;

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