#include <modes/submode_feature_edit.h>

void MouseReactSplitAddFeature::init(ObjectId p_pl, ObjectId p_pts) {
    pl_id = p_pl; pts_id = p_pts; org_vertex= -1;
}

void MouseReactSplitAddFeature::define_gui() {
    if (org_vertex != -1){
        std::string msg = "Origin vertex " + std::to_string(org_vertex) + " selected";
        ImGui::Text(msg.c_str());
        ImGui::Text("Pick a dest vertex to add a feature");
        ImGui::Text("Right pick anywhere to cancel");
    }
}

void trace_feature(Triangles & tri,PolyLine& pl, EdgeAttribute<int>& feat_id,int org,int dest){

    std::vector<int> path;
    PointAttribute<double> dist(tri, 1e20);
    for (auto e : pl.iter_edges()) FOR(lv,2) dist[pl.vert(e,lv)]= -1;
    dist[dest] = 1e20;


    PointAttribute<int> prev(tri, -1);
    auto cmp = [&](int i, int j) {// -1 needs special treatment for getting the first element
        if (i == -1) return true;
        if (j == -1) return false;
        if (dist[i] == dist[j]) return i < j;
        return dist[i] < dist[j];
        };
    std::set<int, decltype(cmp)> heap(cmp);
    dist[org] = 0;
    heap.insert(org);
    while (true) {
        if (heap.empty()) { Log::error("Path does not exist"); return; }
        auto cur = heap.upper_bound(-1);
        Surface::Vertex v(tri, *cur);
        heap.erase(cur);

        if (v == dest) {
            while (v != org) {
                path.push_back(prev[v]);
                v = Surface::Halfedge(tri, prev[v]).from();
            }
            break;
        }

        for (auto cir : v.iter_halfedges()) {
            auto opp_v = cir.to();
            double new_dist = dist[v] + (vec3(v) - vec3(opp_v)).norm();

            if (//prev[opp_v] == -1 || 
                dist[opp_v] > new_dist) {
                dist[opp_v] = new_dist;
                prev[opp_v] = cir;
                heap.insert(opp_v);
            }
        }
    }
    
    int max_feats = 0;
    for (auto e : pl.iter_edges()) max_feats = std::max(max_feats, feat_id[e]);
    int offset = pl.nedges();
    pl.create_edges(2 * path.size());
    FOR(e, path.size()){
        Surface::Halfedge h(tri, path[e]);
        feat_id[offset + 2 * e] = max_feats+1;
        feat_id[offset + 2 * e+1] = max_feats+1;
        pl.vert(offset + 2 * e, 0) = h.from();
        pl.vert(offset + 2 * e, 1) = h.to();
        pl.vert(offset + 2 * e+1, 1) = h.from();
        pl.vert(offset + 2 * e+1, 0) = h.to();
    }

}

void MouseReactSplitAddFeature::on_click(int button, vec2 p) {
    if (button == 1 && org_vertex != -1) {
        org_vertex = -1;
        return;
    }

    Picker picker;
    Picker::Result pr = picker.at(p);
    int v_selected = pr.filtered_primitive_id(pts_id, 0);
    if (v_selected != -1){
        PolyLine& pl = HexModelerMode::geom_edge_id();
        PolyLineAttributes& attr = HexModelerMode::geom_edge_id();
        EdgeAttribute<int> feat_id("feat_id", attr, pl);
        if (button == 0){
            if (org_vertex == -1) {
                org_vertex = v_selected;
                return;
            }
            Triangles& tri = HexModelerMode::geom_tri_id();
            trace_feature(tri,pl, feat_id, org_vertex, v_selected);
            org_vertex = -1;
            HexModelerMode::geom_edge_id().broadcast(UPDATED);
        }
        if (button == 1){// split feature
            DisjointSet ds(pl.nedges());
            for (auto e : pl.iter_edges()){
                auto opp = e.opposite();
                if (opp.active() && feat_id[opp]== feat_id[e]) ds.merge(opp, e);
            }
            for (auto v : pl.iter_vertices()) {
                if (v_selected!=v)
                for (auto e0 : v.iter_edges())for (auto e1 : v.iter_edges()){
                    if (feat_id[e0] == feat_id[e1]) ds.merge(e0,e1);
                }
            }
            std::vector<int> new_feat;
            ds.get_sets_id(new_feat);
            for (auto e : pl.iter_edges()) feat_id[e] = new_feat[e];
            HexModelerMode::geom_edge_id().broadcast(UPDATED);
        }
    }
}







FeatureEditMode::FeatureEditMode(HexModelerMode& mode) : HexModelerMode::SubMode(mode){
    multi_selector_react_left.init(0);
    multi_selector_react_right.init(1);
    split_add_react.init(geom_edge_id(), geom_pts_id());
}
void FeatureEditMode::define_gui() {
    ImGui::Begin("HexModelerMode", nullptr);
    ImGui::Text("MODE: FeatureEditMode");
    split_add_react.define_gui();
    multi_selector_react_left.feed_back_gui();
    multi_selector_react_right.feed_back_gui();
    ImGui::End();
}

void FeatureEditMode::activate(){
    Log::add("FeatureEditMode::activate()");
    God::layers["meshPts"].visible = false;
    God::layers["meshHex"].visible = false;
    God::layers["meshEdges"].visible = false;
    God::layers["geomEdges"].bind_attribute(ObjectId(ObjectType::POLYLINES_ATTR, { "geom", "","int","edges","feat_id" }));
    auto& er = static_cast<PolyLineLayer&>(God::layers["geomEdges"]).edge_renderer;
    FOR(i, 2) er.data_range[i] = er.data_autorange[i];
    static_cast<PointSetLayer&>(God::layers["geomPts"]).primitive_renderer.radius_in_pixel = 7;
}

void FeatureEditMode::handle(Event event) {
    // forward mouse events to mouse reacts 
    if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
        mode.handle_camera_and_layerselect(event);
    else {
        send_instruction_to_react(event, multi_selector_react_left);
        send_instruction_to_react(event, multi_selector_react_right);
        send_instruction_to_react(event, split_add_react);
    }

    // consume mouse react "return values"
    PolyLine& pl = geom_edge_id();
    PolyLineAttributes& attr = geom_edge_id();
    EdgeAttribute<int> feat_id("feat_id", attr, pl);


    if (multi_selector_react_left.done){//==> split a layer
        Log::add("CONSUME multi_selector_react");
        auto crossed_edges = multi_selector_react_left.primitives(geom_edge_id(), 1);
        if (crossed_edges.empty()){
            // do nothing
        }
        else if (crossed_edges.size() == 1){// split id
            int prim = crossed_edges.front().first;
            vec3 I = crossed_edges.front().second;
            vec3 P[2]; FOR(i, 2) P[i] = pl.points[pl.vert(prim, i)];
            double c = (P[1] - P[0]) * (I - P[0]) / (P[1] - P[0]).norm2();
            plop("need to split");
            //c = std::clamp(c, .1, .9);
        }
        else  {// merge ids
            int new_feat_id = feat_id[crossed_edges.front().first];
            std::vector<int> feat2merge;
            auto id_in_feat2merge = [&](int feat){FOR(i, feat2merge.size()) if (feat2merge[i] == feat) return i; return -1; };

            for (auto it : crossed_edges)
                if (id_in_feat2merge(feat_id[it.first]) == -1)
                    feat2merge.push_back(feat_id[it.first]);

            bool has_branch = false;
            DisjointSet ds(feat2merge.size());
            for (auto v : pl.iter_vertices()){
                int nincident = 0;
                for (auto e : v.iter_edges()){
                    int id = id_in_feat2merge(feat_id[e]);
                    if (id != -1) nincident++;
                }
                has_branch = has_branch || nincident > 2;

                for (auto e0 : v.iter_edges()){
                    int id0 = id_in_feat2merge(feat_id[e0]);
                    if (id0 == -1) continue;
                    for (auto e1 : v.iter_edges()){
                        int id1 = id_in_feat2merge(feat_id[e1]);
                        if (id1 == -1) continue;
                        ds.merge(id0, id1);
                    }
                }
            }
            std::vector<int> trash;
            int nconnex_components = ds.get_sets_id(trash);

            if (nconnex_components == 1 && !has_branch) for (auto it : crossed_edges) {
                int f = feat_id[it.first];
                for (auto e : pl.iter_edges()) if (feat_id[e] == f) feat_id[e] = new_feat_id;
                geom_edge_id().broadcast(UPDATED);
            }
        }

        multi_selector_react_left.reset();
    }
    if (multi_selector_react_right.done){//==> remove a feature
        Log::add("CONSUME multi_selector_react RIGHT");
        auto crossed_edges = multi_selector_react_right.primitives(geom_edge_id(), 1);
        if (!crossed_edges.empty()){
            std::vector<bool> to_kill(pl.nedges(), false);
            for (auto it : crossed_edges) for (auto e : pl.iter_edges()) to_kill[e] = to_kill[e] || (feat_id[it.first] == feat_id[e]);
            pl.disconnect();
            pl.delete_edges(to_kill);
            pl.connect();
            geom_edge_id().broadcast(UPDATED);
        }
        multi_selector_react_right.reset();
    }

}