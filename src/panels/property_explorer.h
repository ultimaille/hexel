#include "core/event.h"
struct PropertyExplorer : public Panel {
    std::vector<ObjectId> layers;
    void generate_gui(){
        ImGui::SetNextWindowPos(ImVec2(10, 720), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250, 300), ImGuiCond_Always);
        ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        for (ObjectId& id : layers){
            ImGui::Separator();
            ImGui::Text(id.chunks.back().c_str());
            RenderLayer& layer=id;
            layer.generate_gui(id.chunks.back());        
        }
        ImGui::End();
    }
};