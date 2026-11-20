#include <core/all.h>
#include <layers/layer_ui.h>
#include "property_explorer.h"

void PropertyExplorer::generate_gui(){
        ImGui::Begin("Properties", nullptr);

        static std::unordered_map<std::string, bool> advanced_open;
        static std::unordered_map<std::string, bool> attributes_open;

        for (Path& id : layers){
            if (!id.valid()) continue;
            Layer &layer = id;
            auto layer_name = id.path.back(); // TODO maybe change that
            ImGui::PushID(layer_name.c_str());

            // ImGui::Separator();
            // ImGui::Checkbox("##visible", &layer.visible);
            // ImGui::SameLine();
            // ImGui::TextUnformatted(layer_name.c_str());


            std::unique_ptr<LayerUI> layer_ui;

            if (PointSetLayer *pl = dynamic_cast<PointSetLayer*>(&layer)) {
                layer_ui = std::make_unique<PointSetLayerUI>(*pl);
            } else if (PolyLineLayer *pl = dynamic_cast<PolyLineLayer*>(&layer)) {
                layer_ui = std::make_unique<PolyLineLayerUI>(*pl);
            } else if (TrianglesLayer *pl = dynamic_cast<TrianglesLayer*>(&layer)) {
                layer_ui = std::make_unique<TrianglesLayerUI>(*pl);
            } else if (HexahedraLayer *pl = dynamic_cast<HexahedraLayer*>(&layer)) {
                layer_ui = std::make_unique<HexahedraLayerUI>(*pl);
            } else {
                ImGui::PopID();
                continue;
            }

            // Basic parameters
            layer_ui->show_parameters(LayerUI::BASIC);

            // Advanced parameters
            DrawThinSection2("advanced", "advanced", advanced_open[layer_name], [&layer_ui=layer_ui]() {
                layer_ui->show_parameters(LayerUI::ADVANCED);
            });
            // Attribute parameters
            DrawThinSection2("attributes", "attributes", attributes_open[layer_name], [&layer_ui=layer_ui]() {
                layer_ui->show_parameters(LayerUI::ATTRIBUTE);
            });

            ImGui::PopID();
        }

        // Keep this for compatibility, i don't know its purpose
        std::vector<Path> nv_layers;
        for (Path& id : layers){
            if (!id.valid()) continue;
            nv_layers.push_back(id);
        }

        ImGui::End();
        layers = nv_layers;
    }
