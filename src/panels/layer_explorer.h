struct LayerExplorer : public Panel {
    void generate_gui(){
        ImGui::SetNextWindowPos(ImVec2(10, 410), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(250, 700), ImGuiCond_Always);
        ImGui::Begin("LayersConfig", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        std::vector<std::string> to_kill;

        int drag_from = -1;
        int drop_to = -1;
        for (int i = 0; i < God::layers.size(); ++i){
            RenderLayer& layer = God::layers[i];
            std::string layer_name = God::layers.ith_name(i);


            bool tree_open = ImGui::TreeNodeEx(layer_name.c_str(), ImGuiTreeNodeFlags_DrawLinesFull);
            ImGui::SameLine(); ImGui::Checkbox(("on##visible" + layer_name).c_str(), &layer.visible);
            ImGui::SameLine(); if (ImGui::Button(("kill##" + layer_name).c_str())){
                to_kill.push_back(layer_name);
            }
            if (tree_open){
                layer.generate_gui(layer_name);
                ImGui::TreePop();
            }
            // The widget is the drag source
            if (ImGui::BeginDragDropSource()){
                ImGui::SetDragDropPayload("WIDGET", &i, sizeof(i));
                ImGui::Text("Moving %s", God::layers.ith_name(i));
                ImGui::EndDragDropSource();
            }

            // The same widget is the drop target
            if (ImGui::BeginDragDropTarget()){
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WIDGET")){
                    drag_from = *(const int*)payload->Data;
                    drop_to = i;
                }
                ImGui::EndDragDropTarget();
            }
        }
        if (drag_from != -1 || drop_to != -1) {
            while (drag_from < drop_to) {
                God::layers.swap(drag_from, drag_from + 1);
                drag_from++;
            }
            while (drag_from > drop_to) {
                God::layers.swap(drag_from, drag_from - 1);
                drag_from--;
            }
        }
        for (std::string layer_name : to_kill){
            God::layers.kill(layer_name);
        }
        ImGui::End();
    }
};