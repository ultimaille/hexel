#include <modes/submode_feature_edit.h>




FeatureEditMode::FeatureEditMode(HexModelerMode& mode) : HexModelerMode::SubMode(mode){
    multi_selector_react_left.init(0);
    multi_selector_react_right.init(1);
    split_add_react.init(geom_edge_id(), geom_pts_id());
}
void FeatureEditMode::define_gui() {
    ImGui::Begin("HexModelerMode", nullptr);
    ImGui::Text("MODE: FeatureEditMode");
    multi_selector_react_left.feed_back_gui();
    multi_selector_react_right.feed_back_gui();
    ImGui::End();
}

void FeatureEditMode::activate(){
    Log::add("FeatureEditMode::activate()");
    God::layers["meshPts"].visible = false;
    God::layers["meshHex"].visible = false;
    God::layers["meshEdges"].visible = false;
    God::layers["geomEdges"].bind_attribute(ObjectId(ObjectType::POLYLINES_ATTR, { "geom", "","int","edges","feat_id"}));
    auto& er = static_cast<PolyLineLayer&>(God::layers["geomEdges"]).edge_renderer;
    FOR(i, 2) er.data_range[i] = er.data_autorange[i];
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
    EdgeAttribute<int> feat_id("feat_id", attr,pl);


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
                if (id_in_feat2merge(feat_id[it.first])==-1)
                    feat2merge.push_back(feat_id[it.first]);
            
            bool has_branch=false;
            DisjointSet ds(feat2merge.size());
            for (auto v : pl.iter_vertices()){
                int nincident=0;
                for (auto e : v.iter_edges()){
                    int id = id_in_feat2merge(feat_id[e]);
                    if (id != -1) nincident++;
                }
                has_branch = has_branch || nincident >2;

                 for (auto e0 : v.iter_edges()){
                    int id0 = id_in_feat2merge(feat_id[e0]);
                    if (id0==-1) continue;
                    for (auto e1 : v.iter_edges()){
                        int id1 = id_in_feat2merge(feat_id[e1]);
                        if (id1 == -1) continue;
                        ds.merge(id0,id1);
                    }
                }
            }
            std::vector<int> trash;
            int nconnex_components= ds.get_sets_id(trash);
            
            if (nconnex_components ==1 && !has_branch) for (auto it : crossed_edges) {
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
            std::vector<bool> to_kill(pl.nedges(),false);
            for (auto it : crossed_edges) for (auto e : pl.iter_edges()) to_kill[e] = to_kill[e] || (feat_id[it.first] == feat_id[e]);
            pl.disconnect();
            pl.delete_edges(to_kill);
            pl.connect();
            geom_edge_id().broadcast(UPDATED);
        }
        multi_selector_react_right.reset();
    }

}