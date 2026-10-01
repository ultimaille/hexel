struct LayerExplorer : public Panel {
    void generate_gui(){
        // ImGui::SetNextWindowPos(ImVec2(10, 410), ImGuiCond_Always);
        // ImGui::SetNextWindowSize(ImVec2(250, 700), ImGuiCond_Always);
        // ImGui::Begin("LayersConfig", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Begin("LayersConfig", nullptr);








        static std::vector<bool> pressed;
        pressed.resize(God::layers.size(), false);

        std::vector<std::string> to_kill;

        int drag_from = -1;
        int drop_to = -1;

        for (int i = 0; i < God::layers.size(); ++i){
            RenderLayer& layer = God::layers[i];
            std::string layer_name = God::layers.ith_name(i);


            bool was_pressed = pressed[i];
            if (pressed[i]){
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.6f, 0.15f, 1.0f));
            }
            if (ImGui::Button(label(layer_name,"proppanel")))
                pressed[i] = !pressed[i];
            if (was_pressed) ImGui::PopStyleColor(3);
            // The widget is the drag source
            if (ImGui::BeginDragDropSource()){
                ImGui::SetDragDropPayload("WIDGET", &i, sizeof(i));
                ImGui::Text("Moving %s", (God::layers.ith_name(i)).c_str());
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
            ImGui::SameLine();
            float largeur = ImGui::GetContentRegionAvail().x;
            float largeurWidget = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + largeur - largeurWidget);

            ImGui::Checkbox(label("##visible", layer_name), &layer.visible);
            ImGui::SameLine(); if (ImGui::Button(label("X##", layer_name))){
                to_kill.push_back(layer_name);
            }
        }
        if (drag_from != -1 || drop_to != -1) {
            while (drag_from < drop_to) {
                God::layers.swap(drag_from, drag_from + 1);
                bool tmp= pressed[drag_from];
                pressed[drag_from]= pressed[drag_from+1];
                pressed[drag_from+1]= tmp;

                //std::swap(pressed[drag_from], pressed[drag_from +1]);
                drag_from++;
            }
            while (drag_from > drop_to) {
                God::layers.swap(drag_from, drag_from - 1);
                bool tmp = pressed[drag_from];
                pressed[drag_from] = pressed[drag_from - 1];
                pressed[drag_from -1] = tmp;
                drag_from--;
            }
        }
        for (std::string layer_name : to_kill){
            God::layers.kill(layer_name);
        }

        {// update the set of current layers in the property windows
            ObjectId id({ chunk_panel,"property_window"});
            if (id.ptr() != nullptr){
                PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>( id));

                pan.layers.clear();
                for (int i = 0; i < God::layers.size(); ++i){
                    if (pressed[i])
                        pan.layers.push_back(ObjectId({ chunk_layer, God::layers.ith_name(i)}));
                }
            }
            else Log::add("Property_window not found");
        }
        ImGui::End();
    }
};