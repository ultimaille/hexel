#include "layers/basic_layers.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <set>

using namespace events;

bool FilePopup(const char* id, std::string& out, std::vector<const char*> extensions) {
    static std::filesystem::path path, sel;
    static bool init = false;
    bool done = false;

    if (!ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))return false;

    if (!init) { path = out; sel.clear(); init = true; }

    ImGui::TextUnformatted(path.string().c_str());
    if (ImGui::Button("[..]") && path.has_parent_path())    path = path.parent_path(), sel.clear();
    ImGui::SameLine();
    if (ImGui::Button("LoadMultiMesh##fileselector"))              out = sel.string(), done = true;
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))                            out = "", done = true;


    ImGui::BeginChild("files", { 400,250 }, true);
    for (auto& e : std::filesystem::directory_iterator(path)) {
        auto p = e.path();
        bool d = e.is_directory();

        if (!d) {
            bool filtered = true;
            for (auto ext : extensions)if (p.extension() == std::string(ext)) filtered = false;
            if (filtered) continue;
        }
        std::string n = (d ? "[D] " : "") + p.filename().string();
        ImGui::Selectable(n.c_str(), p == sel);
        if (ImGui::IsItemClicked()) sel = p;
        if (d && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))  path = p, sel.clear();
        if (!d && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) out = p.string(), done = true;
    }
    ImGui::EndChild();
    if (done)ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return done;
}




//void object_id_drag_source(ObjectId id, std::string name) {
//    if (ImGui::BeginDragDropSource()) {
//        ObjectId* ptr = new ObjectId(id);
//        ImGui::SetDragDropPayload("OBJECTID", &ptr, sizeof(ObjectId*));
//        ImGui::Text("Moving %s", name.c_str());
//        ImGui::EndDragDropSource();
//    }
//}

struct XCFExplorer : public Panel {

    void look_at_pointset(PointSet& ps) {
        BBox3 box;
        FOR(v, ps.size()) box.add(ps[v]);
        TrackBallCamera& cam = dynamic_cast<TrackBallCamera&>(*God::camera.impl);
        cam.pose.pivot = box.center();
        cam.projection.view_height = box.size()[1];
    }

    void load_mm_with_default_layers(std::string path) {
        auto mesh_name = God::xcf.load_multimesh(path, true);
        look_at_pointset(ObjectId(POINTSET, mesh_name));

        {// point set
            RenderSpheres& layer = God::layers.add<RenderSpheres>(mesh_name + "Pts");
            layer.init(ObjectId(POINTSET, mesh_name));
            auto& pr = layer.primitive_renderer;
            pr.color[0] = .5;
            pr.color[1] = 1.;
            pr.color[2] = .5;
            pr.color_map_prop = 0.0;
            pr.ambient_prop = .5;
            pr.radius_in_pixel = 4;
        }
        for (auto& elt : God::xcf[mesh_name].polylines) {
            RenderTubes& layer = God::layers.add<RenderTubes>(mesh_name + "Edges");
            layer.init(ObjectId(POLYLINES, {mesh_name, elt.first}));
            auto& pr = layer.primitive_renderer;
            pr.color[0] = .5;
            pr.color[1] = .5;
            pr.color[2] = .7;
            pr.color_map_prop = 0.0;
            pr.ambient_prop = .5;
        }
        for (auto& elt : God::xcf[mesh_name].triangles) {
            RenderLambertTriangles& layer = God::layers.add<RenderLambertTriangles>(mesh_name + "Tri");
            layer.init(ObjectId(TRIANGLES, {mesh_name, elt.first}));
            auto& pr = layer.primitive_renderer;
            pr.color[0] = .8;
            pr.color[1] = .8;
            pr.color[2] = .8;
            pr.color_map_prop = 0.0;
            pr.ambient_prop = .5;
        }
        for (auto& elt : God::xcf[mesh_name].quads) {
            RenderLambertQuads& layer = God::layers.add<RenderLambertQuads>(mesh_name + "Quad");
            layer.init(ObjectId(QUADS, {mesh_name, elt.first}));
            auto& pr = layer.primitive_renderer;
            pr.color[0] = .8;
            pr.color[1] = .8;
            pr.color[2] = .8;
            pr.color_map_prop = 0.0;
            pr.ambient_prop = .5;
        }
        for (auto& elt : God::xcf[mesh_name].tetrahedra) {
            RenderLambertTet& layer = God::layers.add<RenderLambertTet>(mesh_name + "Tet");
            layer.init(ObjectId(TETRAHEDRA, {mesh_name, elt.first}));
            auto& pr = layer.primitive_renderer;
            pr.color[0] = .8;
            pr.color[1] = .8;
            pr.color[2] = .8;
            pr.color_map_prop = 0.0;
            pr.ambient_prop = .5;
        }
    }


    void generate_gui() {
        static std::set<ObjectId, std::function<bool(ObjectId, ObjectId)>> selected ([&](const ObjectId& a, const ObjectId& b) { if (a.path != b.path) return a.path < b.path; return a.names < b.names; });

        static ObjectId dnd_obj;
        auto object_id_drag_source = [](ObjectId id, std::string name) {
            if (ImGui::BeginDragDropSource()) {
                //ObjectId* ptr = new ObjectId(id);
                dnd_obj = id;
                ObjectId* ptr = &dnd_obj;
                ImGui::SetDragDropPayload("OBJECTID", & ptr, sizeof(ObjectId*));
                //ImGui::SetDragDropPayload("OBJECTID", &ptr, sizeof(ObjectId*));
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
            if (FilePopup("FilePopup", path, { ".mesh" ,".geogram" })) {
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
                ImGui::Separator;
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
                ImGui::Separator;
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

        //std::vector<std::string> mm_to_kill;
        //for (auto& [mm_name, mm] : God::xcf) {
        //	static bool closable_mm_group = true;
        //	if (ImGui::CollapsingHeader(mm_name.c_str(),&closable_mm_group)) {


        //		if (ImGui::TreeNode(label("pointset",mm_name))) {
        //			//... show shaders
        //			ImGui::TreePop();
        //		}
        //		if (!mm.polylines.empty()) if (ImGui::TreeNode(label("polylines", mm_name))) {
        //			//... show shaders
        //			ImGui::TreePop();
        //		}


        //		{
        //			std::vector<ObjectId> to_kill;
        //			for (auto& [tri_name, obj] : mm.triangles) {
        //				static bool closable_mesh_group = true;
        //				ObjectId id({ chunk_xcf,mm_name,"triangles",tri_name });
        //				if (ImGui::CollapsingHeader(label(tri_name, mm_name+"tri"), &closable_mesh_group)) {
        //					// render layers
        //					for(int i=0;i< God::layers.size();i++){
        //						if (God::layers[i].require(id)) {
        //							God::layers[i].generate_gui(God::layers.ith_name(i));
        //						}
        //					}
        //				}

        //				if (ImGui::Button(label("kill odd triangles", mm_name+tri_name))) {
        //					Triangles& tri = obj.mesh;
        //					std::vector<bool> to_kill(tri.nfacets(), false);
        //					for (auto f : tri.iter_facets()) to_kill[f] = (f % 2) == 0;
        //					tri.disconnect();
        //					tri.delete_facets(to_kill);
        //					tri.connect();
        //					id.emit(UPDATED);
        //				}

        //				// delete if the cross is pressed
        //				if (!closable_mesh_group) {
        //					to_kill.push_back(id);
        //					closable_mesh_group = true;
        //				}
        //			}
        //			for (auto id : to_kill) {
        //				id.show();
        //				God::xcf.kill_mesh(id);
        //			}


        //			{// create new triangles
        //				static char new_tri_name[64] = "newtri";
        //				if (ImGui::Button(label("Add new", mm_name))) {
        //					std::string s = new_tri_name;
        //					God::xcf.add_triangles(mm_name, s);
        //					//ImGui::TreePop();
        //					break;
        //				}
        //				ImGui::SameLine(); ImGui::InputText(label("##edit", mm_name), new_tri_name, 64);
        //			}

        //			//ImGui::TreePop();
        //		}
        //		if (!mm.quads.empty()) if (ImGui::TreeNode(label("quads", mm_name))) {
        //			//... show shaders
        //			ImGui::TreePop();
        //		}

        //	}
        //	if (!closable_mm_group) {
        //		mm_to_kill.push_back(mm_name);
        //		closable_mm_group = true;
        //	}
        //}
        //for (auto name : mm_to_kill)
        //	God::xcf.kill_multimesh(name);



        ImGui::End();
    }
};
