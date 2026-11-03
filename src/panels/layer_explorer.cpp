#include "layer_explorer.h"
#include "property_explorer.h"
#include <core/layers.h>
#include <core/event.h>

using namespace events;

// sync pressed with the property windows
void LayerExplorer::sync_layer_from_property() {
    layer_pressed = std::vector<bool>(God::layers.size(), false);

    ObjectId id(ObjectType::PANEL, "property_window");
    
    if (id.ref() == std::nullopt) {
        Log::add("Property_window not found");
        return;
    }

    PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(id));
    for (auto id : pan.layers)
        if (God::layers.contains(id.names.back()))
            layer_pressed[God::layers.find(id.names.back())] = true;
}

void LayerExplorer::sync_property_from_layer() {
    ObjectId id(PANEL, "property_window");
    PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(id));

    pan.layers.clear();
    for (int i = 0; i < God::layers.size(); ++i){
        if (layer_pressed[i])
            pan.layers.push_back(ObjectId(LAYER, God::layers.ith_name(i)));
    }
}

void LayerExplorer::reorder_layers(int from, int to) {
    
    // Eventually change the layer group 
    auto &layer_from = God::layers[from];
    auto &layer_to = God::layers[to];
    int old_group = layer_from.group;
    int working_group = layer_to.group;
    layer_from.group = working_group;

    if (God::layers.is_group_empty(old_group))
        God::layers.remove_group(old_group);

    int step = (from < to) ? 1 : -1;

    for (int i = from; i != to; i += step) {
        if (God::layers[i].group != working_group)
            continue;

        int next = i + step;
        God::layers.swap(i, next);
        
        bool tmp = layer_pressed[i];
        layer_pressed[i] = layer_pressed[next];
        layer_pressed[next] = tmp;
    }
}

void LayerExplorer::draw_layer_gui() {

    sync_layer_from_property();

    std::vector<std::string> to_kill;

    ImGui::Begin("LayersConfig", nullptr);

    for (int i = 0; i < God::layers.groups.size(); ++i) {
        auto &group = God::layers.groups[i];

        ImGui::PushID(("widget_group_" + std::to_string(group.id)).c_str());

        bool open = ImGui::TreeNodeEx(label(group.name, "group_" + std::to_string(group.id)), flag(false, false));

        // Button to open popup
        static bool showPopup = false;
        static int selectedItem = -1;

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            ImGui::OpenPopup("blend_context");
        }

        // Popup with selection list
        if (ImGui::BeginPopup("blend_context")) {
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
            
            // Drop to swap two groups
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NODE_INDEX")) {
                int source = *(int*)payload->Data;
                if (source != i && source >= 0 && source < (int)God::layers.groups.size()) {
                    std::swap(God::layers.groups[source], God::layers.groups[i]);
                }
            }

            // Drop to move layer into another group
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_NODE_INDEX")) {
                int source = *(int*)payload->Data;
                if (source >= 0 && source < (int)God::layers.size()) {
                    auto &layer = God::layers[source];
                    int old_group = layer.group;
                    layer.group = God::layers.groups[i].id;
                    // Eventually remove group
                    if (God::layers.is_group_empty(old_group))
                        God::layers.remove_group(old_group);
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (open) {
            for (int l = 0; l < God::layers.size(); ++l) {
                ImGui::PushID(("tree_node_layer_" + std::to_string(l)).c_str());

                Layer &layer = God::layers[l];
                if (layer.group == group.id) {
                    std::string layer_name = God::layers.ith_name(l);

                    // Apply pressed color style
                    bool was_pressed = layer_pressed[l];
                    if (layer_pressed[l]){
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.15f, 0.6f, 0.15f, 1.0f));
                    }

                    bool open_layer_node = ImGui::TreeNodeEx(label(layer_name, layer_name), flag(false, true));

                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                        ImGui::OpenPopup("layer_context");
                    }

                    if (ImGui::BeginPopup("layer_context")) {
                        if (ImGui::Button("Delete")) {
                            to_kill.push_back(layer_name);
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    if (was_pressed) ImGui::PopStyleColor(3);

                    // Drag layer
                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload("LAYER_NODE_INDEX", &l, sizeof(int));
                        std::string layer_name = God::layers.ith_name(l);
                        ImGui::Text("%s", ("Move layer " + layer_name).c_str());
                        ImGui::EndDragDropSource();
                    }


                    bool was_dropped = false;
                    // Drop target reorder layer
                    if (ImGui::BeginDragDropTarget()) {
                        int drag_from = -1;
                        int drop_to = -1;

                        // Drop to move layer
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_NODE_INDEX")) {
                            int source = *(int*)payload->Data;
                            if (source >= 0 && source < (int)God::layers.size()) {
                                auto &source_layer = God::layers[source];
                                drag_from = *(const int*)payload->Data;
                                drop_to = l;
                            }
                        }

                        // Reorder
                        if (drag_from != -1 || drop_to != -1)
                            reorder_layers(drag_from, drop_to);

                        ImGui::EndDragDropTarget();
                        was_dropped = true;
                    }

                    if (!was_dropped && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
                        layer_pressed[l] = !layer_pressed[l];

                    
                    ImGui::SameLine(0.f, 4.f);
                    ImGui::Checkbox(label("", "cb_layer_visible"), &layer.visible);

                    // ImGui::SameLine(); 
                    // if (ImGui::SmallButton(label("X##", layer_name))){
                    //     to_kill.push_back(layer_name);
                    // }

                    if (open_layer_node) {
                        ImGui::TreePop();
                    }
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }

        sync_property_from_layer();

        ImGui::PopID();

        for (std::string layer_name : to_kill){
            God::layers.kill(layer_name);
        }
    }

    bool new_group_open = ImGui::TreeNodeEx(label("+", "group_new"), flag(false, true));
    // Drop target
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("LAYER_NODE_INDEX")) {
            int source = *(int*)payload->Data;
            if (source >= 0 && source < (int)God::layers.size()) {
                int new_group_id = God::layers.add_group("misc");
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