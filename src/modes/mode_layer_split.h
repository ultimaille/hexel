#include "misc/purgatory.h"
#include "layers/ssao.h"
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>


void generate_edge_graph(Hexahedra& hex, PolyLine& pl, EdgeAttribute<int>& hex_halfedge,bool remove_isolated_vertices = true) {
    std::vector<bool> to_kill(pl.nedges(), true);
    pl.delete_edges(to_kill);
    if (remove_isolated_vertices)
        hex.delete_isolated_vertices();
    if (!hex.connected()) hex.connect();
    DisjointSet ds(hex.nhalfedges());
    for (auto h : hex.iter_halfedges())
        if (h.facet().opposite().active())
            ds.merge(h, h.opposite_c().opposite_f());

    std::vector<int> edges_;
    int nedges = ds.get_sets_id(edges_);
    pl.create_edges(nedges);
    hex_halfedge.fill(- 1);
    for (auto h : hex.iter_halfedges()) {
        int e = edges_[h];
        pl.vert(e, 0) = h.from();
        pl.vert(e, 1) = h.to();
        hex_halfedge[e] = h;
    }
    pl.connect();
}

struct MouseReactLayerSelection : public MouseReact {
    MouseReactLayerSelection() {}
    void init(ObjectId p_hex, ObjectId p_pl, ObjectId p_pts) { hex_id = p_hex; pl_id = p_pl; pts_id = p_pts; }
    ObjectId hex_id;
    ObjectId pl_id;
    ObjectId pts_id;

    void on_click(int button, vec2 p) {
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
            PointSet&  pts = pts_id;
            PointAttribute<bool> visible_pts("visible", pts_attr, pts);
            visible_pts.fill(false);
            for (auto c : hex.iter_cells()) if (visible_cell[c])
                FOR(lv,8)visible_pts[c.vertex(lv)] = true;


            PolyLineAttributes& pl_attr = pl_id;
            PolyLine& pl = pl_id;
            EdgeAttribute<bool> visible_edge("visible", pl_attr, pl);
            visible_edge.fill(false);
            for (auto e : pl.iter_edges()) if (visible_pts[e.from()] && visible_pts[e.to()] )
                visible_edge[e] = true;

            hex_id.broadcast(EventType::UPDATED);
            pl_id.broadcast(EventType::UPDATED);
            pts_id.broadcast(EventType::UPDATED);
        }
    }
};

struct MouseReactMultiSelector : public MouseReact {
    MouseReactMultiSelector() {}

    void init(int button){ 
        active_button = button;
    }
    
    void reset(){
        path.clear();
        picked.clear();
        done =false;
    }

    // filter the primitives
    std::vector<std::pair<int,vec3> > primitives(ObjectId mesh_id, int primitive_type){
        std::vector <std::pair<int, vec3>> res;
        for (auto pr : picked) {
            int prim = pr.filtered_primitive_id(mesh_id, primitive_type);
            if (prim != -1) res.push_back({ prim,pr.point() });
        }
        return res;
    }

    void on_click(int button, vec2 p) {
        if (button != active_button) return;
        Picker::Result pr = picker.at(p);
        picked.push_back(pr);
        done = true;
    }

    void on_press(int button, vec2 p){
        if (button != active_button) return;
        picker.reset(); 
        reset();
        path.push_back(p);
    }

    void on_drag(int button, vec2 a, vec2 b){
        if (button != active_button) return;
        path.push_back(b);
    }

    void on_release(int button, vec2 p){
        if (button != active_button) return;
        path.push_back(p);
        if (path.size() < 2) return;
        FOR(s, path.size() - 1) for (vec2 P:iter_raster_line(path[s], path[s+1])){
            Picker::Result pr = picker.at(P);
            bool already_in = false;
            for (auto pr_iter : picked)
                already_in = already_in || (pr.layer_id == pr_iter.layer_id && pr.raw_primitive_id == pr_iter.raw_primitive_id);
            if (!already_in) picked.push_back(pr);
        }
        done = true;
    }

    void feed_back_gui(){
        std::vector<ImVec2> pts;
        for (auto v : path)pts.push_back(ImVec2(v.x, v.y));
        auto col = IM_COL32(127, 255, 255, 255);
        if (active_button==1) col = IM_COL32(255, 255, 127, 255);
        ImGui::GetBackgroundDrawList()->AddPolyline(pts.data(), pts.size(), col, ImDrawFlags_JoinMiter, 5.0f);
        FOR(i, 10){
            int decal = std::max(0, int(path.size()) - i);
            ImGui::GetBackgroundDrawList()->AddPolyline(pts.data() + decal, int(pts.size()) - decal, col, ImDrawFlags_JoinMiter, 15.0f - float(i));
        }
    }

    Picker picker;
    int active_button=0;
    std::vector<vec2> path;
    std::vector<Picker::Result> picked;
    bool done = false;
};



struct MouseReactSplitMergeLayers : public MouseReact {
    MouseReactSplitMergeLayers() {}

    void init(ObjectId p_mesh) { mesh_id = p_mesh; }
    ObjectId mesh_id;

    void on_click(int button, vec2 p) {
        Log::add("click " + std::to_string(button));
        Picker picker;
        Picker::Result pr = picker.at(p);
        PolyLine& pl = mesh_id;
        int prim = pr.filtered_primitive_id(mesh_id, 1);
        if (prim != -1){
            int v0 = pl.vert(prim, 0);
            int v1 = pl.vert(prim, 1);
            vec3 A = pl.points[v0];
            vec3 B = pl.points[v1];
            vec3 P = pr.point();
            double c = (B - A) * (P - A) / (B - A).norm2();
            plop(c);
            if (button == 0) plop(prim);
            if (button == 1) plop(prim);
            mesh_id.broadcast(EventType::UPDATED);
        }
    }
};

#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")


struct LayerSplitMode : public Mode{

    MouseReactTrackBallCamera camera_react;
    MouseReactSplitMergeLayers split_merge_react;
    MouseReactLayerSelection layer_selection_react;
    MouseReactMultiSelector multi_selector_react_left;
    MouseReactMultiSelector multi_selector_react_right;


    inline ObjectId hex_id(){ return ObjectId(ObjectType::HEXAHEDRA, std::vector<std::string>{ "mesh", ""}); }
    inline ObjectId pl_id(){ return ObjectId(ObjectType::POLYLINES, std::vector<std::string>{ "mesh", ""}); };
    inline ObjectId pts_id(){ return ObjectId(ObjectType::POINTSET, std::vector<std::string>{ "mesh"}); };

    EdgeAttribute<int> edge2halfedge;



    Volume::Halfedge hex_halfedge_from_edge(int e){
        Hexahedra& hex = hex_id();
        return hex.halfedge(edge2halfedge[e]);
    }

    LayerSplitMode(){
        import_new_project_from_CAD(std::string(TEST_INPUT_DIR) + "B1.geogram");
        load_mm_with_default_layers(std::string(WORKING_DIR) + "geom");
        load_mm_with_default_layers(std::string(WORKING_DIR) + "mesh");

        Hexahedra& hex = hex_id();
        God::xcf["mesh"].polylines.clear();
        God::xcf["mesh"].polylines.try_emplace("");
        PolyLine& pl = pl_id();
        pl.points = hex.points;
        edge2halfedge.bind("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
        generate_edge_graph(hex, pl, edge2halfedge);

        PolyLineLayer& layer = God::layers.add<PolyLineLayer>("mesh_edge");
        layer.init(pl_id());
        auto& pr = layer.primitive_renderer;        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;

        layer_selection_react.init(hex_id(),pl_id(),pts_id());
        split_merge_react.init(pl_id());
        multi_selector_react_left.init(0);
        multi_selector_react_right.init(1);
    }



    bool remove_hex_layer(Hexahedra& hex, CellFacetAttribute<bool>& kill_layer){
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

    void update_hex(){
        

        generate_edge_graph(hex_id(), pl_id(), edge2halfedge);
        hex_id().broadcast(EventType::UPDATED);
        pl_id().broadcast(EventType::UPDATED);
        pts_id().broadcast(EventType::UPDATED);
    }


    void handle(Event event) {
        // forward mouse events to mouse reacts 
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL)){
            send_instruction_to_react(event, camera_react);
            send_instruction_to_react(event, layer_selection_react);
        }
        else {
            send_instruction_to_react(event, split_merge_react);
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


    void import_new_project_from_CAD(std::string filename){
        BBox3 bbox;
        {
            MultiMesh mm;
            mm.load_geogram(filename, false);
            FOR(v, mm.pointset.size()) bbox.add(mm.pointset[v]);

            if (mm.polylines.empty()) mm.polylines.try_emplace("");
            auto& pl = mm.polylines[""];
            pl.attributes.points.clear();
            pl.attributes.edges.clear();
            EdgeAttribute<int> feature("feat_id", pl.attributes, pl.mesh);
            for (auto e : pl.mesh.iter_edges()) feature[e] = e;

            if (mm.triangles.empty()) mm.triangles.try_emplace("");
            auto& tri = mm.triangles[""];
            tri.attributes.points.clear();
            tri.attributes.corners.clear();
            tri.attributes.facets.clear();
            FacetAttribute<int> region("region_id", tri.attributes, tri.mesh);
            for (auto f : tri.mesh.iter_facets()) region[f] = f;

            mm.quads.clear(); mm.polygons.clear(); mm.hexahedra.clear(); mm.pyramids.clear(); mm.wedges.clear();
            mm.save_to_path(WORKING_DIR + "geom");

        }

        {
            MultiMesh mm;
            mm.hexahedra.try_emplace("");
            CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);
            //bbox
            mm.pointset.create_points(8);
            Hexahedra &hex = mm.hexahedra[""].mesh;
            hex.create_cells(1);
            FOR(k, 2)FOR(j, 2)FOR(i, 2) mm.pointset[i + 2 * j + 4 * k] = bbox.min + vec3(i * bbox.size()[0], j * bbox.size()[1], k * bbox.size()[2]);
            FOR(v, 8) hex.vert(0, v) = v;
            
            mm.save_to_path(WORKING_DIR + "mesh");
        }

    }


    void define_gui() {
        ImGui::Begin("Mode", nullptr);
        ImGui::Text("Window used to launch debug tests");
        multi_selector_react_left.feed_back_gui();
        multi_selector_react_right.feed_back_gui();
        ImGui::End();
    }
};
