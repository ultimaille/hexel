#include <core/all.h>
#include "property_explorer.h"
void PropertyExplorer::generate_gui(){
        ImGui::Begin("Properties", nullptr);
        for (ObjectId& id : layers){
            ImGui::Separator();
            ImGui::Text(id.names.back().c_str());
            Layer& layer = id;
            layer.generate_gui(id.names.back());
        }
        ImGui::End();
    }
