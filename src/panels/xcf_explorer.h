#include "misc/purgatory.h"
#include <imgui.h>
#include <string>
#include <set>
#include <core/panels.h>

using namespace events;

struct XCFExplorer : public Panel {
    typedef std::set<Path, std::function<bool(Path, Path)>> PathSet;

    int flag(bool is_selected, bool leaf = false) {
        return ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanLabelWidth |
            ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_NavLeftJumpsToParent |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_DrawLinesFull |
            (is_selected ? ImGuiTreeNodeFlags_Selected : 0) |
            (leaf ? ImGuiTreeNodeFlags_Leaf : 0);
    };

    void object_id_drag_source(Path id, std::string name);

    void switch_selection(PathSet& selected, Path object_id){
        if (selected.contains(object_id)) selected.erase(object_id);
        else selected.insert(object_id);
    };

    bool open_mesh(PathSet& selected, MeshPath mesh, std::string category);

    void add_mesh_layers(PathSet& selected, MeshPath mesh_id, std::string gna);

    void add_attribute(PathSet& selected, MeshPath mesh_id, AttributePath attr_id, std::string name);
    void generate_gui();

    struct NodeColor{
        void init(ImVec4 base, ImVec4 hovered, ImVec4 active){
            ImGui::PushStyleColor(ImGuiCol_Header, base);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hovered);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, active);
        }
        NodeColor(int style_id = 0){
            switch (style_id){
            case 0: init(ImVec4(0.2, 0.4, 0.8, 1.0), ImVec4(.3, .5, .9, 1.0), ImVec4(.1, .3, .7, 1.0)); return;
            case 1: init(ImVec4(0.5, 0.4, 0.8, 1.0), ImVec4(.6, .5, .9, 1.0), ImVec4(.4, .3, .7, 1.0)); return;
            case 2: init(ImVec4(0.2, 0.8, 0.4, 1.0), ImVec4(.3, .9, .5, 1.0), ImVec4(.1, .7, .1, 1.0)); return;
            case 3: init(ImVec4(0.2, 0.8, 0.8, 1.0), ImVec4(.3, .9, .9, 1.0), ImVec4(.1, .7, .7, 1.0)); return;
            };
            init(ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0), ImVec4(1.0, 1.0, 1.0, 1.0));
        }
        NodeColor(ImVec4 base, ImVec4 hovered, ImVec4 active){
            init(base, hovered, active);
        }
        ~NodeColor(){
            ImGui::PopStyleColor(3);
        }
    };
};
