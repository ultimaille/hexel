#include <core/all.h>
#include "property_explorer.h"
void PropertyExplorer::generate_gui(){
        ImGui::Begin("Properties", nullptr);
        std::vector<Path> nv_layers;
        for (Path& id : layers){
            if (!id.valid()) continue;
            nv_layers.push_back(id);
            ImGui::Separator();
            ImGui::TextUnformatted(id.path.back().c_str());
            Layer& layer = id;
            layer.generate_gui(id.path.back());
        }
        ImGui::End();
        layers = nv_layers;
    }
