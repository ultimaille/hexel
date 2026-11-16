#pragma once
#include <core/all.h>
#include <algo/raster_iterator.h>

struct HexModelerSubMode;

struct InteractorLayerSelection : public Interactor {
    InteractorLayerSelection(HexModelerSubMode& mode):hexmode(mode) {}
    HexModelerSubMode &hexmode;

    void on_click(int button, vec2 p);
};

struct InteractorMultiSelector : public Interactor {
    InteractorMultiSelector() {}

    inline void init(int button){
        active_button = button;
    }

    inline void reset(){
        path.clear();
        picked.clear();
        done = false;
    }

    // filter the primitives
    inline std::vector<std::pair<int, vec3> > primitives(MeshPath mesh_id, int primitive_type){
        std::vector <std::pair<int, vec3>> res;
        for (auto pr : picked) {
            int prim = pr.filtered_primitive_id(mesh_id, primitive_type);
            if (prim != -1) res.push_back({ prim,pr.point() });
        }
        return res;
    }

    inline void on_click(int button, vec2 p) {
        if (!key_state_is_OK()) return;
        if (button != active_button) return;
        Picker::Result pr = picker.at(p);
        picked.push_back(pr);
        done = true;
    }

    inline void on_press(int button, vec2 p){
        if (!key_state_is_OK()) return;
        if (button != active_button) return;
        picker.reset();
        reset();
        path.push_back(p);
    }

    inline void on_drag(int button, vec2 a, vec2 b){
        if (!key_state_is_OK()) return;
        if (button != active_button) return;
        path.push_back(b);
    }

    inline void on_release(int button, vec2 p){
        if (!key_state_is_OK()) return;
        if (button != active_button) return;
        path.push_back(p);
        if (path.size() < 2) return;
        FOR(s, path.size() - 1) for (vec2 P : iter_raster_line(path[s], path[s + 1])){
            Picker::Result pr = picker.at(P);
            bool already_in = false;
            for (auto pr_iter : picked)
                already_in = already_in || (pr.layer_id == pr_iter.layer_id && pr.raw_primitive_id == pr_iter.raw_primitive_id);
            if (!already_in) picked.push_back(pr);
        }
        done = true;
    }

    inline void feed_back_gui(){
        if (!key_state_is_OK()) return;
        std::vector<ImVec2> pts;
        for (auto v : path)pts.push_back(ImVec2(v.x, v.y));
        auto col = IM_COL32(127, 255, 255, 255);
        if (active_button == 1) col = IM_COL32(255, 255, 127, 255);
        ImGui::GetBackgroundDrawList()->AddPolyline(pts.data(), pts.size(), col, ImDrawFlags_JoinMiter, 5.0f);
        FOR(i, 10){
            int decal = std::max(0, int(path.size()) - i);
            ImGui::GetBackgroundDrawList()->AddPolyline(pts.data() + decal, int(pts.size()) - decal, col, ImDrawFlags_JoinMiter, 15.0f - float(i));
        }
    }

    Picker picker;
    int active_button = 0;
    std::vector<vec2> path;
    std::vector<Picker::Result> picked;
    bool done = false;
};
