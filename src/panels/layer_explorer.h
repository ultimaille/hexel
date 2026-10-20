struct LayerExplorer : public Panel {
    void generate_gui(){
        ImGui::Begin("LayersConfig", nullptr);

        std::vector<bool> pressed(God::layers.size(), false);

        {// sync pressed with the property windows
            ObjectId id(PANEL, "property_window");
            if (id.ref() != std::nullopt) {
                PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(id));
                for (auto id : pan.layers)
                    if (God::layers.contains(id.names.back()))
                        pressed[God::layers.find(id.names.back())] = true;
            }
            else Log::add("Property_window not found");
        }

        std::vector<std::string> to_kill;

        int drag_from = -1;
        int drop_to = -1;

        for (int i = 0; i < God::layers.size(); ++i){
            Layer& layer = God::layers[i];
            std::string layer_name = God::layers.ith_name(i);


            bool was_pressed = pressed[i];
            if (pressed[i]){
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.6f, 0.15f, 1.0f));
            }
            if (ImGui::Button(label(layer_name, "proppanel")))
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
                bool tmp = pressed[drag_from];
                pressed[drag_from] = pressed[drag_from + 1];
                pressed[drag_from + 1] = tmp;

                //std::swap(pressed[drag_from], pressed[drag_from +1]);
                drag_from++;
            }
            while (drag_from > drop_to) {
                God::layers.swap(drag_from, drag_from - 1);
                bool tmp = pressed[drag_from];
                pressed[drag_from] = pressed[drag_from - 1];
                pressed[drag_from - 1] = tmp;
                drag_from--;
            }
        }
        for (std::string layer_name : to_kill){
            God::layers.kill(layer_name);
        }

        {// update the set of current layers in the property windows
            ObjectId id(PANEL, "property_window");
            PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(id));

            pan.layers.clear();
            for (int i = 0; i < God::layers.size(); ++i){
                if (pressed[i])
                    pan.layers.push_back(ObjectId(LAYER, God::layers.ith_name(i)));
            }
        }


        for (int i = 0; i < God::layers.groups.size(); ++i) {
            auto &group = God::layers.groups[i];

            ImGui::PushID(("widget_group_" + std::to_string(group.id)).c_str());

            bool open = ImGui::TreeNodeEx(label("Group " + std::to_string(group.id), "group_" + std::to_string(group.id)));
            

            // Button to open popup
            static bool showPopup = false;
            static int selectedItem = -1;
            // ImGui::SameLine();
            // if (ImGui::SmallButton(label("+", "btn_merge_" + std::to_string(group.id)))) {
            //     ImGui::OpenPopup("SelectionListPopup");
            //     showPopup = true;
            // }
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                ImGui::OpenPopup("MergeSelectionListPopup");
            }

            // Popup with selection list
            if (ImGui::BeginPopup("MergeSelectionListPopup")) {
                ImGui::Text("Select merge option:");
                ImGui::Separator();
                
                static const char* items[] = {"None", "Average"};
                
                for (int i = 0; i < IM_ARRAYSIZE(items); i++) {
                    if (ImGui::Selectable(items[i], i == group.blend_op)) {
                        group.blend_op = static_cast<Group::BlendOp>(i);
                        // Do something with the selected item
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndPopup();
            }


            // Drag source
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("NODE_INDEX", &i, sizeof(int));
                ImGui::Text("%s", ("Move group " + std::to_string(group.id)).c_str());
                ImGui::EndDragDropSource();
            }
            
            // Drop target (reorder within same level)
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NODE_INDEX")) {
                    int source = *(int*)payload->Data;
                    if (source != i && source >= 0 && source < (int)God::layers.groups.size()) {
                        std::swap(God::layers.groups[source], God::layers.groups[i]);
                    }
                }
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_NODE_INDEX")) {
                    int source = *(int*)payload->Data;
                    if (source >= 0 && source < (int)God::layers.size()) {
                        God::layers[source].group = i;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (open) {
                for (int l = 0; l < God::layers.size(); ++l) {
                    Layer &layer = God::layers[l];
                    if (layer.group == group.id) {
                        std::string layer_name = God::layers.ith_name(l);
                        bool open_layer_node = ImGui::TreeNodeEx(label(layer_name, layer_name));

                        // Drag source
                        if (ImGui::BeginDragDropSource()) {
                            ImGui::SetDragDropPayload("LAYER_NODE_INDEX", &l, sizeof(int));
                            std::string layer_name = God::layers.ith_name(l);
                            ImGui::Text("%s", ("Move layer " + layer_name).c_str());
                            ImGui::EndDragDropSource();
                        }

                        if (open_layer_node) {
                            ImGui::TreePop();
                        }
                    }
                }
                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        bool new_group_open = ImGui::TreeNodeEx(label("+", "group_new"));
        // Drop target
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_NODE_INDEX")) {
                int source = *(int*)payload->Data;
                if (source >= 0 && source < (int)God::layers.size()) {
                    int new_group_id = God::layers.add_group();
                    God::layers[source].group = new_group_id;
                }
            }
            ImGui::EndDragDropTarget();
        }
        if (new_group_open) {
            ImGui::TreePop();
        }


        ImGui::End();
    }
};
