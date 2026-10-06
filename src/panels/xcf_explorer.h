#include "layers/basic_layers.h"
#include "misc/purgatory.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <set>

using namespace events;







struct XCFExplorer : public Panel {


    void generate_gui() {
        static std::set<ObjectId, std::function<bool(ObjectId, ObjectId)>> selected ([&](const ObjectId& a, const ObjectId& b) { if (a.path != b.path) return a.path < b.path; return a.names < b.names; });

        static ObjectId dnd_obj;
        auto object_id_drag_source = [](ObjectId id, std::string name) {
            if (ImGui::BeginDragDropSource()) {
                dnd_obj = id;
                ObjectId* ptr = &dnd_obj;
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

        {// load new mm
            static std::string path;
            if (ImGui::Button("Add New MultiMesh")) {
                if (path.empty()) path = std::string(TEST_INPUT_DIR);
                ImGui::OpenPopup("FilePopup");
            }
            if (FilePopup("FilePopup",std::string(TEST_INPUT_DIR), path, { ".mesh" ,".geogram" })) {
                if (!path.empty())
                    load_mm_with_default_layers(path);
            }
        }








        auto flag = [&](ObjectId cur, bool leaf = false) {
            bool is_selected = selected.contains(cur);
            return ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanAvailWidth |
                ImGuiTreeNodeFlags_OpenOnDoubleClick |
                ImGuiTreeNodeFlags_DrawLinesFull |
                (is_selected ? ImGuiTreeNodeFlags_Selected : 0) |
                (leaf ? ImGuiTreeNodeFlags_Leaf : 0);
            };



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


        // ==> MultiMesh
        for (auto& [mm_name, mm] : God::xcf) {

            auto add_mesh = [&](std::string mesh_name, ObjectType  mesh_type, std::string gna){
                ImGui::Separator();
                ObjectId mesh_id(mesh_type, {mm_name,mesh_name});
                bool open_mesh = ImGui::TreeNodeEx(label(gna + "." + mesh_name, mm_name), flag(mesh_id, false));
                if (ImGui::IsItemClicked()) switch_selection(mesh_id);
                object_id_drag_source(mesh_id, mm_name+"."+mesh_name);

                if (!open_mesh) return;
                for (int i = 0; i < God::layers.size(); i++)
                    if (God::layers[i].require(mesh_id))
                        add_switch_button(ObjectId(LAYER, God::layers.ith_name(i)));
                ImGui::TreePop();
            };

            static bool closable_mm_group = true;
            ObjectId mm_id(MULTIMESH, mm_name);   
            bool open_mm = (ImGui::TreeNodeEx(label(mm_name, "mm"), flag(mm_id)));
            object_id_drag_source(mm_id, mm_name);


            if (ImGui::IsItemClicked()) switch_selection(mm_id);
            if (open_mm){
                // ==> pointset
                ImGui::Separator();
                ObjectId mesh_id(POINTSET, mm_name);
                bool open_mesh = ImGui::TreeNodeEx(label("pointset", mm_name), flag(mesh_id, true));
                object_id_drag_source(mesh_id, mm_name+".pointset");

                if (ImGui::IsItemClicked()) switch_selection(mesh_id);
                ImGui::TreePop();
                for (int i = 0; i < God::layers.size(); i++)
                    if (God::layers[i].require(mesh_id))
                        add_switch_button(ObjectId(LAYER, God::layers.ith_name(i)));


                // ==> polylines
                for (auto& [mesh_name, obj] : mm.polylines) add_mesh(mesh_name, POLYLINES, "polylines");

                // ==> triangles
                for (auto& [mesh_name, obj] : mm.triangles) add_mesh(mesh_name, TRIANGLES, "triangles");
                
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
