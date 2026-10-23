#pragma once
#include <misc/purgatory.h>
#include <layers/ssao.h>
#include <core/attribute.h>
#include "algo.h"

struct MouseReactPickerTest2 : public MouseReact {
    MouseReactPickerTest2() {  }

    void init(ObjectId p_attr_id, ObjectId p_mesh) {
        attr_id = p_attr_id;
        mesh_id = p_mesh;
    }
    ObjectId attr_id;
    ObjectId mesh_id;
    FacetAttribute<int> flag;

    
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

            auto layer_opt = God::layers.find_by_id(result.layer_id);
            if (layer_opt.has_value()) {
                auto &layer = layer_opt.value().get();
                auto mesh_id = layer.mesh;
                Triangles &mesh = mesh_id;
                SurfaceAttributes &attr = mesh_id;
                FacetAttribute<int> fa("flag", attr, mesh);
                if (result.raw_primitive_id != -1) {
                    fa[result.raw_primitive_id] = 1;
                }

                // Beuuuurk !!!!
                // Je convertis en object id pour bind ???!!!
                // ObjectId flag_id(ObjectType::TRIANGLES_ATTR, { "hexski", "","int","facets","flag" });
                // layer.bind_attribute(ObjectId{});

                auto attribute_opt = Attribute::getAttribute(attr, "facets.flag");
                if (attribute_opt.has_value()) {
                    auto a = attribute_opt.value();
                    std::cout << "attribute name: " << a.name << std::endl;
                    std::cout << "attribute kind: " << Attribute::elementKindToString(a.kind) << std::endl;
                    std::cout << "attribute type: " << Attribute::elementTypeToString(a.type) << std::endl;
                }
            }

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