#include "misc/purgatory.h"
#include "layers/ssao.h"
#include "algo.h"
#include "pillowing.h"


void generate_edge_graph(Hexahedra& hex, PolyLine& pl, EdgeAttribute<int>& hex_halfedge) {
    std::vector<bool> to_kill(pl.nedges(), true);
    pl.delete_edges(to_kill);
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
    void init(ObjectId p_mesh) { mesh_id = p_mesh; }
    ObjectId mesh_id;
    void on_click(int button, vec2 p) {
        if (button != 0) return;
        Picker picker;
        Picker::Result pr = picker.at(p);
        int prim = pr.filtered_primitive_id(mesh_id, 2);
        if (prim != -1){
            VolumeAttributes& attr = mesh_id;
            Hexahedra& hex = mesh_id;
            CellAttribute<bool> visible("visible", attr, hex);
            CellFacetAttribute<int> layer_id(hex);
            compute_hex_layer_id(hex, layer_id);
            visible.fill(false);
            for (auto f : hex.iter_facets()) if (layer_id[f] == layer_id[prim])
                visible[f.cell()] = true;
            mesh_id.broadcast(EventType::UPDATED);
        }                                                                                          
    }
};

struct MouseReactMultiSelector : public MouseReact {
    MouseReactMultiSelector() {}

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
        if (button != 0) return;
        Picker::Result pr = picker.at(p);
        picked.push_back(pr);
        done = true;
    }

    void on_press(int button, vec2 p){
        if (button != 0) return;
        picker.reset(); 
        reset();
        path.push_back(p);
    }
    void on_drag(int button, vec2 a, vec2 b){
        if (button != 0) return;
        path.push_back(b);
    }

    void on_release(int button, vec2 p){
        if (button != 0) return;
        path.push_back(p);
        if (path.size() < 2) return;
        FOR(s, path.size() - 1)for (vec2 P = path[s]; (P - path[s+1]).norm2()>2; P += (path[s+1] - path[s]).normalized()) {
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
        ImGui::GetBackgroundDrawList()->AddPolyline(pts.data(), pts.size(), IM_COL32(127, 255, 255, 255), ImDrawFlags_JoinMiter, 5.0f);
        FOR(i, 10){
            int decal = std::max(0, int(path.size()) - i);
            ImGui::GetBackgroundDrawList()->AddPolyline(pts.data() + decal, int(pts.size()) - decal, IM_COL32(127, 255, 255, 255), ImDrawFlags_JoinMiter, 15.0f - float(i));
        }
    }

    Picker picker;
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
    MouseReactMultiSelector multi_selector_react;
    

    Volume::Halfedge hex_halfedge_from_edge(int e){
        Hexahedra& hex = hex_id();
        EdgeAttribute<int> hex_halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
        return hex.halfedge(hex_halfedge[e]);
    }

    LayerSplitMode(){
        import_new_project_from_CAD(std::string(TEST_INPUT_DIR) + "B1.geogram");
        load_mm_with_default_layers(std::string(WORKING_DIR) + "geom");
        load_mm_with_default_layers(std::string(WORKING_DIR) + "mesh");

        Hexahedra& hex = God::xcf["mesh"].hexahedra[""].mesh;
        God::xcf["mesh"].polylines.clear();
        God::xcf["mesh"].polylines.try_emplace("");
        PolyLine& pl = God::xcf["mesh"].polylines[""].mesh;
        pl.points = hex.points;
        EdgeAttribute<int> hex_halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
        generate_edge_graph(hex, pl, hex_halfedge);

        PolyLineLayer& layer = God::layers.add<PolyLineLayer>("mesh_edge");
        ObjectId pl_id(POLYLINES, std::vector<std::string>{ "mesh", ""});
        layer.init(pl_id);
        auto& pr = layer.primitive_renderer;
        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;

        ObjectId hex_id(ObjectType::HEXAHEDRA, std::vector<std::string>{ "mesh", ""});
        layer_selection_react.init(hex_id);
        split_merge_react.init(pl_id);
    }

    inline ObjectId hex_id(){ return ObjectId(ObjectType::HEXAHEDRA, std::vector<std::string>{ "mesh", ""}); }
    inline ObjectId hex_pl_id(){ return ObjectId(ObjectType::POLYLINES, std::vector<std::string>{ "mesh", ""}); };




    void handle(Event event) {
        // forward mouse events to mouse reacts 
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL)){
            send_instruction_to_react(event, camera_react);
            send_instruction_to_react(event, layer_selection_react);
        }
        else {
            send_instruction_to_react(event, split_merge_react);
            send_instruction_to_react(event, multi_selector_react);
        }

        // consume mouse react "return values"
        if (multi_selector_react.done){
            Log::add("CONSUME multi_selector_react");
            auto crossed_edges = multi_selector_react.primitives(hex_pl_id(), 1);
            for (auto it : crossed_edges) std::cerr << it.first <<"  "<<it.second<<std::endl;
            if (!crossed_edges.empty()){
                Hexahedra& hex = hex_id();
                PolyLine& pl = hex_pl_id();
                int prim = crossed_edges.front().first;
                vec3 I = crossed_edges.front().second;
                vec3 P[2];FOR(i,2) P[i] = pl.points[pl.vert(prim, i)];
                double c = (P[1] - P[0]) * (I - P[0]) / (P[1] - P[0]).norm2();
                plop(c);
                CellFacetAttribute<int> layer_id(hex);
                compute_hex_layer_id(hex, layer_id);
                auto h = hex_halfedge_from_edge(prim);
                int selected_layer = layer_id[h.prev().opposite_f().facet()];
                CellFacetAttribute<int> pillow(hex,0);
                for (auto f : hex.iter_facets()) 
                    if (selected_layer == layer_id[f]) 
                        pillow[f] = 1;

                Pillow(hex, pillow).apply(1.-c);

                EdgeAttribute<int> hex_halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
                generate_edge_graph(hex, pl, hex_halfedge);

                hex_id().broadcast(EventType::UPDATED);
                hex_pl_id().broadcast(EventType::UPDATED);
            }
            multi_selector_react.reset();
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
        multi_selector_react.feed_back_gui();
        ImGui::End();
    }
};
