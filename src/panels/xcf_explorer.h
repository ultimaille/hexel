#include "layers/basic_layers.h"
#include "misc/purgatory.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <set>

using namespace events;







struct XCFExplorer : public Panel {
    typedef std::set<ObjectId, std::function<bool(ObjectId, ObjectId)>> ObjectIdSet;

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

    void object_id_drag_source(ObjectId id, std::string name) {
        if (ImGui::BeginDragDropSource()) {
            grad_and_drop_objectid = id;
            ObjectId* ptr = &grad_and_drop_objectid;
            ImGui::SetDragDropPayload("OBJECTID", &ptr, sizeof(ObjectId*));
            ImGui::Text("Moving %s", name.c_str());
            ImGui::EndDragDropSource();
        }
    };

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


    void switch_selection(ObjectIdSet& selected, ObjectId object_id){
        if (selected.contains(object_id)) selected.erase(object_id);
        else selected.insert(object_id);
    };

    bool open_mesh(ObjectIdSet& selected, std::string mm_name, std::string mesh_name, ObjectType  mesh_type, std::string gna){
        ImGui::Separator();
        ObjectId mesh_id(mesh_type, { mm_name,mesh_name });
        bool open_mesh;
        {
            NodeColor ncol(1);
            open_mesh = ImGui::TreeNodeEx(label(gna + "." + mesh_name, mm_name), flag(selected.contains(mesh_id), false));
        }

        if (ImGui::IsItemClicked()) switch_selection(selected, mesh_id);
        object_id_drag_source(mesh_id, mm_name + "." + mesh_name);
        return open_mesh;
    };

    void add_mesh_layers(ObjectIdSet& selected, std::string mm_name, std::string mesh_name, ObjectType  mesh_type, std::string gna){
        ObjectId mesh_id(mesh_type, { mm_name,mesh_name });
        for (int i = 0; i < God::layers.size(); i++)
            if (God::layers[i].require(mesh_id)){
                ObjectId layer_id(LAYER, God::layers.ith_name(i));

                bool open_layer;
                {
                    int colorid = 2;
                    if (!selected.contains(layer_id)) colorid = 0;
                    NodeColor ncol(colorid);
                    open_layer = ImGui::TreeNodeEx(label(God::layers.ith_name(i), mm_name), flag(selected.contains(layer_id), true));
                }
                if (open_layer){
                    if (ImGui::IsItemClicked()) switch_selection(selected, layer_id);
                    ImGui::TreePop();
                }
            }
    };
    void close_mesh(){
        ImGui::TreePop();
    };

    void add_attribute(ObjectIdSet& selected, ObjectId mesh_id, ObjectId attr_id, std::string name){
        bool open_attr;
        {
            NodeColor ncol(3);
            open_attr = ImGui::TreeNodeEx(label(name, mesh_id.names[0]), flag(selected.contains(mesh_id), true));
            object_id_drag_source(attr_id, name);
        }
        if (open_attr){
            if (ImGui::IsItemClicked()) switch_selection(selected, attr_id);
            ImGui::TreePop();
        }
    };



    void generate_gui() {
        static ObjectIdSet selected([&](const ObjectId& a, const ObjectId& b) { if (a.path != b.path) return a.path < b.path; return a.names < b.names; });

        {// sync with property panel
            PropertyExplorer& pan = dynamic_cast<PropertyExplorer&>(static_cast<Panel&>(ObjectId(PANEL, "property_window")));
            std::vector<ObjectId> to_kill;
            for (auto sel : selected) if (sel.names.size() == 1 && sel.path == LAYER) to_kill.push_back(sel);
            for (auto id : to_kill)selected.erase(id);
            for (auto id : pan.layers)
                if (!selected.contains(id))
                    selected.insert(id);
        }
        ImGui::Begin("XCFViewer", nullptr);

        //=====================================================================================
        //         CREATE MM
        //=====================================================================================

        if (ImGui::Button(label("New MultiMesh"))) ImGui::OpenPopup("Pick MultiMesh Name##NewMultiMeshPopup");

        ImGui::SetNextWindowPos(ImGui::GetCursorPos());

        if (ImGui::BeginPopupModal("Pick MultiMesh Name##NewMultiMeshPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            static char tmp[1024] = "toto";
            ImGui::InputText(label("", "NewMultiMeshPopup"), tmp, 1024);
            ImGui::SameLine();
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
                God::xcf.add(tmp);
            }
            ImGui::EndPopup();
        }






        //=====================================================================================
        //         START THE TREE
        //=====================================================================================
        std::vector<std::string> mm2kill;

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
            if (ImGui::IsItemClicked()) switch_selection(selected, mm_id);



            if (ImGui::BeginPopupContextItem(label("folder_context", mm_name))){
                if (ImGui::Button("Delete"))
                    mm2kill.push_back(mm_name);

                if (ImGui::BeginMenu("create")){
                    static char tmp[1024] = "toto";
                    ImGui::Text("name"); ImGui::SameLine();
                    ImGui::InputText(label("", "gloglo"), tmp, 1024);

                    if (ImGui::MenuItem(label("Polyline", mm_name))){
                        God::xcf.add_mesh(ObjectId(ObjectType::POLYLINES, { mm_name,tmp }));
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem(label("Triangles", mm_name))){
                        God::xcf.add_mesh(ObjectId(ObjectType::TRIANGLES, { mm_name,tmp }));
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem(label("Quads", mm_name))){
                        God::xcf.add_mesh(ObjectId(ObjectType::QUADS, { mm_name,tmp }));
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem(label("Tetrahedra", mm_name))){
                        God::xcf.add_mesh(ObjectId(ObjectType::TETRAHEDRA, { mm_name,tmp }));
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::MenuItem(label("Hexahedra", mm_name))){
                        God::xcf.add_mesh(ObjectId(ObjectType::HEXAHEDRA, { mm_name,tmp }));
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }


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
                if (ImGui::IsItemClicked()) switch_selection(selected, mesh_id);
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
                                if (ImGui::IsItemClicked()) switch_selection(selected, layer_id);
                                ImGui::TreePop();
                            }
                        }
                    ImGui::TreePop();
                }


                // ==> polylines
                for (auto& [mesh_name, obj] : mm.polylines) {
                    if (open_mesh(selected, mm_name, mesh_name, POLYLINES, "polylines")){
                        for (auto& it : obj.attributes.points) add_attribute(selected, mesh_id,
                            ObjectId(ObjectType::POLYLINES_ATTR, { mm_name, mesh_name ,it.name }),
                            "points." + it.name);
                        for (auto& it : obj.attributes.edges) add_attribute(selected, mesh_id,
                            ObjectId(ObjectType::POLYLINES_ATTR, { mm_name, mesh_name ,it.name }),
                            "edges." + it.name);
                        add_mesh_layers(selected, mm_name, mesh_name, POLYLINES, "polylines");
                        close_mesh();
                    }
                }

                // ==> triangles
                std::vector<std::string> tri2kill;
                for (auto& [mesh_name, obj] : mm.triangles) {
                    bool open = open_mesh(selected, mm_name, mesh_name, TRIANGLES, "triangles");
                    if (ImGui::BeginPopupContextItem(label("folde_context", mm_name + mesh_name))){
                        if (ImGui::Button("Delete")){
                            tri2kill.push_back(mesh_name);
                        ImGui::CloseCurrentPopup();
                    }
                        ImGui::EndPopup();
                    }
                    if (open){
                        for (auto& it : obj.attributes.points) add_attribute(selected, mesh_id,
                            ObjectId(ObjectType::TRIANGLES_ATTR, { mm_name, mesh_name ,it.name }),
                            "points." + it.name);
                        for (auto& it : obj.attributes.corners) add_attribute(selected, mesh_id,
                            ObjectId(ObjectType::TRIANGLES_ATTR, { mm_name, mesh_name ,it.name }),
                            "corners." + it.name);
                        for (auto& it : obj.attributes.facets) add_attribute(selected, mesh_id,
                            ObjectId(ObjectType::TRIANGLES_ATTR, { mm_name, mesh_name ,it.name }),
                            "facets." + it.name);
                        add_mesh_layers(selected, mm_name, mesh_name, TRIANGLES, "triangles");
                        close_mesh();
                    }
                }
                for (auto n : tri2kill) God::xcf.kill_mesh(ObjectId(ObjectType::TRIANGLES, {mm_name,n}));
                ImGui::TreePop();

            }
        }
        for (auto n : mm2kill) God::xcf.kill_multimesh(n);

        {// sync with property panel
            PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(ObjectId(PANEL, "property_window")));
            pan.layers.clear();
            for (auto sel : selected)if (sel.path == LAYER) pan.layers.push_back(sel);
        }



        ImGui::End();
    }
};
