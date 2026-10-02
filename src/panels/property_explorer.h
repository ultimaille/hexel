#include "core/event.h"
struct PropertyExplorer : public Panel {
    std::vector<ObjectId> layers;
    void generate_gui(){
        ImGui::Begin("Properties", nullptr);
        for (ObjectId& id : layers){
            ImGui::Separator();
            ImGui::Text(id.names.back().c_str());
            RenderLayer& layer=id;
            layer.generate_gui(id.names.back());        
        }
        ImGui::End();
    }
};
