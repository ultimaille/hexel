#include <modes/submode_layer_split.h>




    void activate(){}

    LayerSplitMode::LayerSplitMode(HexModelerMode& mode) : HexModelerMode::SubMode(mode){
        multi_selector_react_left.init(0);
        multi_selector_react_right.init(1);
    }

    bool LayerSplitMode::remove_hex_layer(Hexahedra& hex, CellFacetAttribute<bool>& kill_layer){
        PointAttribute<int> map(hex);
        for (auto v : hex.iter_vertices()) map[v] = v;
        std::vector<bool> to_kill(hex.ncells(), false);
        for (auto f : hex.iter_facets()){
            if (!kill_layer[f]) continue;
            to_kill[f.cell()] = true;
            for (auto h : f.iter_halfedges())
                map[h.from()] = h.opposite_f().next().to();
        }
        for (auto v : hex.iter_vertices()) if (map[v] != v && map[map[v]] != map[v]) {
            Log::error("Cannot remove hex layers with Transitivity :(");
            return false;
        }
        hex.disconnect();
        for (auto c : hex.iter_cells()) FOR(lv, 8) hex.vert(c, lv) = map[hex.vert(c, lv)];
        hex.delete_cells(to_kill);
        return true;

    }

    void LayerSplitMode::handle(Event event) {
        // forward mouse events to mouse reacts 
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
            mode.handle_camera_and_layerselect(event);
        else {
            send_instruction_to_react(event, multi_selector_react_left);
            send_instruction_to_react(event, multi_selector_react_right);
        }

        // consume mouse react "return values"
        Hexahedra& hex = hex_id();
        PolyLine& pl = pl_id();

        if (multi_selector_react_left.done){//==> split a layer
            Log::add("CONSUME multi_selector_react");
            auto crossed_edges = multi_selector_react_left.primitives(pl_id(), 1);
            for (auto it : crossed_edges) std::cerr << it.first << "  " << it.second << std::endl;
            if (!crossed_edges.empty()){
                int prim = crossed_edges.front().first;
                vec3 I = crossed_edges.front().second;
                vec3 P[2]; FOR(i, 2) P[i] = pl.points[pl.vert(prim, i)];
                double c = (P[1] - P[0]) * (I - P[0]) / (P[1] - P[0]).norm2();
                c = std::clamp(c,.1,.9);
                CellFacetAttribute<int> layer_id(hex);
                compute_hex_layer_id(hex, layer_id);
                auto h = hex_halfedge_from_edge(prim);
                int selected_layer = layer_id[h.prev().opposite_f().facet()];
                CellFacetAttribute<int> pillow(hex, 0);
                for (auto f : hex.iter_facets())
                    if (selected_layer == layer_id[f])
                        pillow[f] = 1;

                Pillow(hex, pillow).apply(1. - c);

                update_hex();
            }
            multi_selector_react_left.reset();
        }
        if (multi_selector_react_right.done){//==> remove a layer
            Log::add("CONSUME multi_selector_react");
            auto crossed_edges = multi_selector_react_right.primitives(pl_id(), 1);
            for (auto it : crossed_edges) std::cerr << it.first << "  " << it.second << std::endl;
            if (!crossed_edges.empty()){
                int prim = crossed_edges.front().first;
                vec3 I = crossed_edges.front().second;
                vec3 P[2]; FOR(i, 2) P[i] = pl.points[pl.vert(prim, i)];
                double c = (P[1] - P[0]) * (I - P[0]) / (P[1] - P[0]).norm2();
                CellFacetAttribute<int> layer_id(hex);
                compute_hex_layer_id(hex, layer_id);
                auto h = hex_halfedge_from_edge(prim);
                if (c > .5) h = h.opposite_f();
                int selected_layer = layer_id[h.prev().opposite_f().facet()];
                CellFacetAttribute<bool> kill_layer(hex, false);
                for (auto f : hex.iter_facets())
                    if (selected_layer == layer_id[f])
                        kill_layer[f] = true;

                remove_hex_layer(hex, kill_layer);

                update_hex();
            }
            multi_selector_react_right.reset();
        }

    }

    void LayerSplitMode::define_gui() {
            ImGui::Begin("HexModelerMode", nullptr);
            ImGui::Text("MODE: LayerSplitMode");
            multi_selector_react_left.feed_back_gui();
            multi_selector_react_right.feed_back_gui();
            ImGui::End();
    }

