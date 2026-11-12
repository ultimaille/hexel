#include <core/all.h>
#include <layers/layer_ui.h>
#include "property_explorer.h"

void PropertyExplorer::generate_gui(){
        ImGui::Begin("Properties", nullptr);

        
        for (ObjectId& id : layers){
            if (!id.ref().has_value()) continue;
            Layer &layer = id;
            auto layer_name = id.names.back();
            ImGui::PushID(layer_name.c_str());

            ImGui::Separator();
            ImGui::Checkbox("##visible", &layer.visible);
            ImGui::SameLine();
            ImGui::TextUnformatted(layer_name.c_str());

            std::unique_ptr<LayerUI> layer_ui;

            if (PointSetLayer *pl = dynamic_cast<PointSetLayer*>(&layer)) {
                layer_ui = std::make_unique<PointSetLayerUI>(*pl);
            } else if (PolyLineLayer *pl = dynamic_cast<PolyLineLayer*>(&layer)) {
                layer_ui = std::make_unique<PolyLineLayerUI>(*pl);
            } else if (TrianglesLayer *pl = dynamic_cast<TrianglesLayer*>(&layer)) {
                layer_ui = std::make_unique<TrianglesLayerUI>(*pl);
            } else if (HexahedraLayer *pl = dynamic_cast<HexahedraLayer*>(&layer)) {
                layer_ui = std::make_unique<HexahedraLayerUI>(*pl);
            }

            // Basic parameters
            layer_ui->show_parameters(LayerUI::BASIC);

            // Advanced parameters
            if (ImGui::CollapsingHeader("Advanced")) {
                layer_ui->show_parameters(LayerUI::ADVANCED);
            }
            // Attribute parameters
            if (ImGui::CollapsingHeader("Attributes")) {
                layer_ui->show_parameters(LayerUI::ATTRIBUTE);
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
