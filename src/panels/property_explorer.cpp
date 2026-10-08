#include <core/all.h>
#include "property_explorer.h"
void PropertyExplorer::generate_gui(){
        ImGui::Begin("Properties", nullptr);
        std::vector<ObjectId> nv_layers;
        for (ObjectId& id : layers){
            if (!id.ref().has_value()) continue;
            nv_layers.push_back(id);
            ImGui::Separator();
            ImGui::Text(id.names.back().c_str());
            Layer& layer = id;
            layer.generate_gui(id.names.back());
        }
        ImGui::End();
        layers = nv_layers;
    }
