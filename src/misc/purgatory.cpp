#include "misc/purgatory.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <core/all.h>
#include <layers/basic_layers.h>
ObjectId grad_and_drop_objectid;



bool FilePopup(const char* id, std::string in, std::string& out, std::vector<const char*> extensions) {
    static std::filesystem::path path, sel;
    static bool init = false;
    bool done = false;

    if (!ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))return false;
    if (!init) { path = in; sel.clear(); init = true; }

    ImGui::TextUnformatted(path.string().c_str());
    if (ImGui::Button("[..]") && path.has_parent_path())    path = path.parent_path(), sel.clear();
    ImGui::SameLine();
    if (ImGui::Button("Return Directory##fileselector"))              out = sel.string(), done = true;
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




using namespace events;
void look_at_pointset(PointSet& ps) {
    if (ps.size() == 0) return;
    BBox3 box;
    FOR(v, ps.size()) box.add(ps[v]);
    God::camera.pose.pivot = box.center();
    God::camera.projection.view_height = box.size()[1];
}

void load_mm_with_default_layers(std::string path) {
    // TODO rename to mm_name
    auto mm_name = God::xcf.load_multimesh(path, true);

    look_at_pointset(ObjectId(POINTSET, mm_name));

    int group = God::layers.add_group(mm_name);

    {// point set
        PointSetLayer& layer = God::layers.add<PointSetLayer>(mm_name + "Pts", group);
        layer.init(ObjectId(POINTSET, mm_name));
        auto& pr = layer.primitive_renderer;
        pr.color[0] = .5; pr.color[1] = 1.; pr.color[2] = .5;
        pr.radius_in_pixel = 4;
    }
    for (auto& elt : God::xcf[mm_name].polylines) {
        PolyLineLayer& layer = God::layers.add<PolyLineLayer>(mm_name + "Edges", group);
        layer.init(ObjectId(POLYLINES, { mm_name, elt.first }));
        auto& pr = layer.primitive_renderer;
        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;
    }
    for (auto& elt : God::xcf[mm_name].triangles) {
        TrianglesLayer& layer = God::layers.add<TrianglesLayer>(mm_name + "Tri", group);
        layer.init(ObjectId(TRIANGLES, { mm_name, elt.first }));
    }
    for (auto& elt : God::xcf[mm_name].hexahedra) {
        HexahedraLayer& layer = God::layers.add<HexahedraLayer>(mm_name + "Hex", group);
        layer.init(ObjectId(HEXAHEDRA, { mm_name, elt.first }));
    }
}
