#include <modes/submode_feature_edit.h>




FeatureEditMode::FeatureEditMode(HexModelerMode& mode) : HexModelerMode::SubMode(mode){
    multi_selector_react_left.init(0);
    multi_selector_react_right.init(1);
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
    God::layers["meshEdge"].visible = false;
}

void FeatureEditMode::handle(Event event) {
    // forward mouse events to mouse reacts 
    if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
        mode.handle_camera_and_layerselect(event);
    else {
        send_instruction_to_react(event, multi_selector_react_left);
        send_instruction_to_react(event, multi_selector_react_right);
    }

    // consume mouse react "return values"
    PolyLine& pl = feature_edge_id();

    if (multi_selector_react_left.done){//==> split a layer
        Log::add("CONSUME multi_selector_react");
        auto crossed_edges = multi_selector_react_left.primitives(pl_id(), 1);
        for (auto it : crossed_edges) std::cerr << it.first << "  " << it.second << std::endl;
        if (!crossed_edges.empty()){
            int prim = crossed_edges.front().first;
            vec3 I = crossed_edges.front().second;
            vec3 P[2]; FOR(i, 2) P[i] = pl.points[pl.vert(prim, i)];
            double c = (P[1] - P[0]) * (I - P[0]) / (P[1] - P[0]).norm2();
            c = std::clamp(c, .1, .9);
            //CellFacetAttribute<int> layer_id(hex);
            //compute_hex_layer_id(hex, layer_id);
            //auto h = hex_halfedge_from_edge(prim);
            //int selected_layer = layer_id[h.prev().opposite_f().facet()];
            //CellFacetAttribute<int> pillow(hex, 0);
            //for (auto f : hex.iter_facets())
            //    if (selected_layer == layer_id[f])
            //        pillow[f] = 1;

            //Pillow(hex, pillow).apply(1. - c);

            //update_hex();
        }
        multi_selector_react_left.reset();
    }
    if (multi_selector_react_right.done){//==> remove a layer
        Log::add("CONSUME multi_selector_react");
        auto crossed_edges = multi_selector_react_right.primitives(pl_id(), 1);
        for (auto it : crossed_edges) std::cerr << it.first << "  " << it.second << std::endl;
        multi_selector_react_right.reset();
    }

}