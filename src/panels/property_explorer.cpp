#include <core/all.h>
#include <layers/layer_ui.h>
#include "property_explorer.h"
void PropertyExplorer::generate_gui(){
        ImGui::Begin("Properties", nullptr);
        std::vector<ObjectId> nv_layers;
        for (ObjectId& id : layers){
            if (!id.ref().has_value()) continue;
            nv_layers.push_back(id);
            ImGui::Separator();
            ImGui::TextUnformatted(id.names.back().c_str());
            Layer& layer = id;

            ImGui::PushID(id.names.back().c_str());

            LayerUI layer_ui(layer);
            layer_ui.show();

            ImGui::PopID();
        }
        ImGui::End();
        layers = nv_layers;
    }
