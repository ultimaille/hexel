#pragma once
#include "layers/basic_layers.h"
#include <imgui.h>
#include <filesystem>
#include <string>
//#include <set>


bool FilePopup(const char* id, std::string in, std::string& out, std::vector<const char*> extensions) {
    ///plop(out);
    static std::filesystem::path path, sel;
    static bool init = false;
    bool done = false;

    if (!ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))return false;
    //plop("victory");
    if (!init) { path = in; sel.clear(); init = true; }

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




using namespace events;
void look_at_pointset(PointSet& ps) {
    BBox3 box;
    FOR(v, ps.size()) box.add(ps[v]);
    God::camera.pose.pivot = box.center();
    God::camera.projection.view_height = box.size()[1];
}

void load_mm_with_default_layers(std::string path) {
    auto mesh_name = God::xcf.load_multimesh(path, true);
    look_at_pointset(ObjectId(POINTSET, mesh_name));

    {// point set
        RenderSpheres& layer = God::layers.add<RenderSpheres>(mesh_name + "Pts");
        layer.init(ObjectId(POINTSET, mesh_name));
        auto& pr = layer.primitive_renderer;
        pr.color[0] = .5; pr.color[1] = 1.; pr.color[2] = .5;
        pr.radius_in_pixel = 4;
    }
    for (auto& elt : God::xcf[mesh_name].polylines) {
        RenderTubes& layer = God::layers.add<RenderTubes>(mesh_name + "Edges");
        layer.init(ObjectId(POLYLINES, { mesh_name, elt.first }));
        auto& pr = layer.primitive_renderer;
        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;
    }
    for (auto& elt : God::xcf[mesh_name].triangles) {
        RenderLambertTriangles& layer = God::layers.add<RenderLambertTriangles>(mesh_name + "Tri");
        layer.init(ObjectId(TRIANGLES, { mesh_name, elt.first }));
    }
    for (auto& elt : God::xcf[mesh_name].quads) {
        RenderLambertQuads& layer = God::layers.add<RenderLambertQuads>(mesh_name + "Quad");
        layer.init(ObjectId(QUADS, { mesh_name, elt.first }));
    }
    for (auto& elt : God::xcf[mesh_name].tetrahedra) {
        RenderLambertTet& layer = God::layers.add<RenderLambertTet>(mesh_name + "Tet");
        layer.init(ObjectId(TETRAHEDRA, { mesh_name, elt.first }));
    }
}
