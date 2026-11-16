#pragma once
#include <core/panels.h>
#include <set>
#include <core/events.h>

struct LayerExplorer : public Panel {

    int flag(bool is_selected, bool leaf = false) {
        return ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanLabelWidth |
            ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_NavLeftJumpsToParent |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_DrawLinesFull |
            (is_selected ? ImGuiTreeNodeFlags_Selected : 0) |
            (leaf ? ImGuiTreeNodeFlags_Leaf : 0);
    };

    void generate_gui(){
        draw_layer_gui();
    }

    void draw_layer_gui();
    void sync_layer_from_property();
    void sync_property_from_layer();
    void reorder_layers(int from, int to);

    std::vector<bool> layer_pressed;
};
