#include "misc/purgatory.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <core/all.h>
#include <layers/basic_layers.h>
Path grad_and_drop_objectid;



void ImGui_help(const char* desc){
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
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

// Generated
bool SaveFilePopup(const char* id, std::string in, std::string& out, const std::vector<const char*>& extensions) {
    static std::filesystem::path path;
    static char filename[512] = {};
    static bool init = false;

    if (!ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return false;

    if (!init) {
        path = in;
        filename[0] = '\0';
        init = true;
    }

    bool done = false;

    ImGui::TextUnformatted(path.string().c_str());

    if (ImGui::Button("[..]") && path.has_parent_path())
        path = path.parent_path();

    ImGui::SameLine();
    if (ImGui::Button("Save") && filename[0] != '\0') {
        std::filesystem::path file = filename;

        if (file.extension().empty() && !extensions.empty())
            file += extensions.front();

        out = (path / file).string();
        done = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        out.clear();
        done = true;
    }

    ImGui::InputText("Filename", filename, sizeof(filename));

    ImGui::BeginChild("save_files", {400, 250}, true);

    std::error_code ec;
    for (std::filesystem::directory_iterator it(path, ec), end;
         !ec && it != end;
         it.increment(ec))
    {
        const auto& entry = *it;
        const auto& p = entry.path();

        std::error_code type_ec;
        const bool is_dir = entry.is_directory(type_ec);
        if (type_ec)
            continue;

        if (!is_dir && !extensions.empty()) {
            bool allowed = false;
            for (const char* ext : extensions) {
                if (p.extension() == ext) {
                    allowed = true;
                    break;
                }
            }
            if (!allowed)
                continue;
        }

        std::string label = (is_dir ? "[D] " : "") + p.filename().string();

        if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
            if (is_dir) {
                filename[0] = '\0';
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    path = p;
            } else {
                std::snprintf(filename, sizeof(filename), "%s",
                              p.filename().string().c_str());
            }
        }
    }

    ImGui::EndChild();

    if (done) {
        init = false;
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
    return done;
}


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

    look_at_pointset(root.xcf[mm_name].pointset);

    int group = God::layers.add_group(mm_name);

    {// point set
        PointSetLayer& layer = God::layers.add<PointSetLayer>(mm_name + "Pts", group);
        layer.init(root.xcf[mm_name].pointset);
        layer.set_default_values();
    }
    for (auto& elt : God::xcf[mm_name].polylines) {
        PolyLineLayer& layer = God::layers.add<PolyLineLayer>(mm_name + "Edges", group);
        layer.init(root.xcf[mm_name].polylines[elt.first]);
        layer.set_default_values();
    }
    for (auto& elt : God::xcf[mm_name].triangles) {
        TrianglesLayer& layer = God::layers.add<TrianglesLayer>(mm_name + "Tri", group);
        layer.init(root.xcf[mm_name].triangles[elt.first]);
    }
    for (auto& elt : God::xcf[mm_name].hexahedra) {
        HexahedraLayer& layer = God::layers.add<HexahedraLayer>(mm_name + "Hex", group);
        layer.init(root.xcf[mm_name].hexahedra[elt.first]);
    }
}
