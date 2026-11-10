#include <core/all.h>
#include <layers/layer_ui.h>
#include "property_explorer.h"

void PropertyExplorer::generate_gui(){
        ImGui::Begin("Properties", nullptr);

        // Basic parameters
        for (ObjectId& id : layers){
            if (!id.ref().has_value()) continue;
            Layer &layer = id;
            auto layer_name = id.names.back();
            ImGui::PushID(layer_name.c_str());

            ImGui::Separator();
            ImGui::Checkbox("##visible", &layer.visible);
            ImGui::SameLine();
            ImGui::TextUnformatted(layer_name.c_str());

            LayerUI layer_ui(layer);
            layer_ui.show_basic_parameters();
            ImGui::PopID();
        }

        // Attribute parameters
        for (ObjectId& id : layers){
            if (!id.ref().has_value()) continue;
            Layer &layer = id;
            auto layer_name = id.names.back();
            ImGui::PushID(layer_name.c_str());
            LayerUI layer_ui(layer);
            if (ImGui::CollapsingHeader(("Attributes " + layer_name).c_str())) {
                layer_ui.show_attribute_parameters();
            }
            ImGui::PopID();
        }

        // Keep this for compatibility, i don't know its purpose
        std::vector<ObjectId> nv_layers;
        for (ObjectId& id : layers){
            if (!id.ref().has_value()) continue;
            nv_layers.push_back(id);
        }

        ImGui::End();
        layers = nv_layers;
    }
