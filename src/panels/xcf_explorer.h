#include "layers/basic_layers.h"
#include "misc/purgatory.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <set>

using namespace events;







struct XCFExplorer : public Panel {

    int flag (bool is_selected, bool leaf = false) {
        return ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanLabelWidth |
            ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_NavLeftJumpsToParent |
            ImGuiTreeNodeFlags_OpenOnDoubleClick |
            ImGuiTreeNodeFlags_DrawLinesFull |
            (is_selected ? ImGuiTreeNodeFlags_Selected : 0) |
            (leaf ? ImGuiTreeNodeFlags_Leaf : 0);
        };


    void generate_gui() {
        static std::set<ObjectId, std::function<bool(ObjectId, ObjectId)>> selected ([&](const ObjectId& a, const ObjectId& b) { if (a.path != b.path) return a.path < b.path; return a.names < b.names; });

        //static ObjectId ObjectId grad_and_drop_objectid;;
        auto object_id_drag_source = [](ObjectId id, std::string name) {
            if (ImGui::BeginDragDropSource()) {
                grad_and_drop_objectid = id;
                ObjectId* ptr = &grad_and_drop_objectid;
                ImGui::SetDragDropPayload("OBJECTID", & ptr, sizeof(ObjectId*));
                ImGui::Text("Moving %s", name.c_str());
                ImGui::EndDragDropSource();
            }
       };

        {// sync with property panel
            PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(ObjectId(PANEL, "property_window")));
            std::vector<ObjectId> to_kill;
            for (auto sel : selected) if (sel.names.size() == 1 && sel.path == LAYER) to_kill.push_back(sel);
            for (auto id : to_kill)selected.erase(id);
            for (auto id : pan.layers)
                if (!selected.contains(id))
                    selected.insert(id);
        }



        ImGui::Begin("XCFViewer", nullptr);

        auto switch_selection = [&](ObjectId object_id){
            if (selected.contains(object_id)) selected.erase(object_id);
            else selected.insert(object_id);
            };
        auto add_switch_button = [&](ObjectId id){
            bool was_pressed = selected.contains(id);
            if (was_pressed){
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.6f, 0.15f, 1.0f));
            }
            if (ImGui::Button(label(id.names[0], "xcf_window")))
                switch_selection(id);
            if (was_pressed) ImGui::PopStyleColor(3);
            };


        struct NodeColor{
            void init(ImVec4 base, ImVec4 hovered, ImVec4 active){
                ImGui::PushStyleColor(ImGuiCol_Header, base);
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hovered);
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, active);
            }
            NodeColor(int style_id=0){
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


        auto open_mesh = [&](std::string mm_name, std::string mesh_name, ObjectType  mesh_type, std::string gna){
            ImGui::Separator();
            ObjectId mesh_id(mesh_type, { mm_name,mesh_name });
            bool open_mesh;
            {
                NodeColor ncol(1);
                open_mesh = ImGui::TreeNodeEx(label(gna + "." + mesh_name, mm_name), flag(selected.contains(mesh_id), false));
            }

            if (ImGui::IsItemClicked()) switch_selection(mesh_id);
            object_id_drag_source(mesh_id, mm_name + "." + mesh_name);
            return open_mesh; 
        };
        auto add_mesh_layers = [&](std::string mm_name, std::string mesh_name, ObjectType  mesh_type, std::string gna){
            ObjectId mesh_id(mesh_type, { mm_name,mesh_name });
            for (int i = 0; i < God::layers.size(); i++)
                if (God::layers[i].require(mesh_id)){
                    ObjectId layer_id(LAYER, God::layers.ith_name(i));

                    bool open_layer;
                    {
                        NodeColor ncol(2);
                        open_layer = ImGui::TreeNodeEx(label(God::layers.ith_name(i), mm_name), flag(selected.contains(mesh_id), true));
                    }
                    if (open_layer){
                        if (ImGui::IsItemClicked()) switch_selection(layer_id);
                        ImGui::TreePop();
                    }
                }
            };
        auto close_mesh = [](){
            ImGui::TreePop();
          };

        //=====================================================================================
        //         START THE TREE
        //=====================================================================================
        // ==> MultiMesh
        for (auto& [mm_name, mm] : God::xcf) {

            static bool closable_mm_group = true;
            ObjectId mm_id(MULTIMESH, mm_name);   
            bool open_mm;
            {
                NodeColor ncol(0);
                open_mm = (ImGui::TreeNodeEx(label(mm_name, "mm"), flag(selected.contains(mm_id))));
            }
            object_id_drag_source(mm_id, mm_name);
            if (ImGui::IsItemClicked()) switch_selection(mm_id);
            if (open_mm){
                // ==> pointset
                ImGui::Separator();
                ObjectId mesh_id(POINTSET, mm_name);
                bool open_pointset;
                {
                    NodeColor ncol(1);
                    open_pointset = ImGui::TreeNodeEx(label("pointset", mm_name), flag(selected.contains(mesh_id), false));
                }
                object_id_drag_source(mesh_id, mm_name + ".pointset");
                if (ImGui::IsItemClicked()) switch_selection(mesh_id);
                if (open_pointset){
                    for (int i = 0; i < God::layers.size(); i++)
                        if (God::layers[i].require(mesh_id)){
                            ObjectId layer_id(LAYER, God::layers.ith_name(i));
                            bool open_layer;
                            {
                                NodeColor ncol(2);
                                open_layer = ImGui::TreeNodeEx(label(God::layers.ith_name(i), mm_name), flag(selected.contains(mesh_id), true));
                                object_id_drag_source(layer_id, mm_name);
                            }
                            if (open_layer){
                                if (ImGui::IsItemClicked()) switch_selection(layer_id);
                                ImGui::TreePop();
                            }
                        }
                            //add_switch_button(ObjectId(LAYER, God::layers.ith_name(i)));
                    ImGui::TreePop();
                }


                // ==> polylines
                //for (auto& [mesh_name, obj] : mm.polylines) add_mesh(mm_name,mesh_name, POLYLINES, "polylines");

                // ==> triangles
                for (auto& [mesh_name, obj] : mm.triangles) {
                    if (open_mesh(mm_name, mesh_name, TRIANGLES, "triangles")){
                        add_mesh_layers(mm_name, mesh_name, TRIANGLES, "triangles");

                        for (auto& it : obj.attributes.points) ImGui::Button(label("points." + it.name, "tri" + mm_name));
                        for (auto& it : obj.attributes.corners) ImGui::Button(label("corners." + it.name, "tri" + mm_name));
                        
                        
                        auto add_attribute = [&](ObjectId attr_id, std::string name){
                            bool open_attr;
                            {
                                NodeColor ncol(3);
                                open_attr = ImGui::TreeNodeEx(label(name, mm_name), flag(selected.contains(mesh_id), true));
                                object_id_drag_source(attr_id, name);
                            }
                            if (open_attr){
                                if (ImGui::IsItemClicked()) switch_selection(attr_id);
                                ImGui::TreePop();
                            }
                        };

                        for (auto& it : obj.attributes.facets) add_attribute(
                            ObjectId (ObjectType::TRIANGLES_ATTR, { mm_name, mesh_name ,it.name }),
                            "facets." + it.name
                        );
                        close_mesh();
                    }
                }
                ImGui::TreePop();

            }
        }

        {// sync with property panel
            PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(ObjectId(PANEL, "property_window")));
            pan.layers.clear();
            for (auto sel : selected)if (sel.path == LAYER) pan.layers.push_back(sel);
        }



        ImGui::End();
    }
};
