#pragma once

struct MouseReactLayerSelection : public MouseReact {
    MouseReactLayerSelection() {}
    void init(ObjectId p_hex, ObjectId p_pl, ObjectId p_pts) { hex_id = p_hex; pl_id = p_pl; pts_id = p_pts; }
    ObjectId hex_id;
    ObjectId pl_id;
    ObjectId pts_id;

    inline void on_click(int button, vec2 p) {
        if (button != 0) return;
        Picker picker;
        Picker::Result pr = picker.at(p);
        int prim = pr.filtered_primitive_id(hex_id, 2);
        if (prim != -1){
            Hexahedra& hex = hex_id;
            CellFacetAttribute<int> layer_id(hex);
            compute_hex_layer_id(hex, layer_id);

            VolumeAttributes& hex_attr = hex_id;
            CellAttribute<bool> visible_cell("visible", hex_attr, hex);
            visible_cell.fill(false);
            for (auto f : hex.iter_facets()) if (layer_id[f] == layer_id[prim])
                visible_cell[f.cell()] = true;
            hex_id.broadcast(EventType::UPDATED);


            PointSetAttributes& pts_attr = pts_id;
            PointSet& pts = pts_id;
            PointAttribute<bool> visible_pts("visible", pts_attr, pts);
            visible_pts.fill(false);
            for (auto c : hex.iter_cells()) if (visible_cell[c])
                FOR(lv, 8)visible_pts[c.vertex(lv)] = true;


            PolyLineAttributes& pl_attr = pl_id;
            PolyLine& pl = pl_id;
            EdgeAttribute<bool> visible_edge("visible", pl_attr, pl);
            visible_edge.fill(false);
            for (auto e : pl.iter_edges()) if (visible_pts[e.from()] && visible_pts[e.to()])
                visible_edge[e] = true;

            hex_id.broadcast(EventType::UPDATED);
            pl_id.broadcast(EventType::UPDATED);
            pts_id.broadcast(EventType::UPDATED);
        }
    }
};

struct MouseReactMultiSelector : public MouseReact {
    MouseReactMultiSelector() {}

    inline void init(int button){
        active_button = button;
    }

    inline void reset(){
        path.clear();
        picked.clear();
        done = false;
    }

    // filter the primitives
    inline std::vector<std::pair<int, vec3> > primitives(ObjectId mesh_id, int primitive_type){
        std::vector <std::pair<int, vec3>> res;
        for (auto pr : picked) {
            int prim = pr.filtered_primitive_id(mesh_id, primitive_type);
            if (prim != -1) res.push_back({ prim,pr.point() });
        }
        return res;
    }

    inline void on_click(int button, vec2 p) {
        if (button != active_button) return;
        Picker::Result pr = picker.at(p);
        picked.push_back(pr);
        done = true;
    }

    inline void on_press(int button, vec2 p){
        if (button != active_button) return;
        picker.reset();
        reset();
        path.push_back(p);
    }

    inline void on_drag(int button, vec2 a, vec2 b){
        if (button != active_button) return;
        path.push_back(b);
    }

    inline void on_release(int button, vec2 p){
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
