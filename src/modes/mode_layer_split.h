#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/generic_mouse_reacts.h>




#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")

struct HexModelerMode : public Mode{

    MouseReactTrackBallCamera camera_react;
    MouseReactLayerSelection layer_selection_react;
    std::string working_directory = WORKING_DIR;
    std::string project_directory = WORKING_DIR;
    std::string CAD_file = std::string(TEST_INPUT_DIR) + "B1.geogram";


    inline ObjectId hex_id(){ return ObjectId(ObjectType::HEXAHEDRA, std::vector<std::string>{ "mesh", ""}); }
    inline ObjectId pl_id(){ return ObjectId(ObjectType::POLYLINES, std::vector<std::string>{ "mesh", ""}); };
    inline ObjectId pts_id(){ return ObjectId(ObjectType::POINTSET, std::vector<std::string>{ "mesh"}); };

    EdgeAttribute<int> edge2halfedge;

    Volume::Halfedge hex_halfedge_from_edge(int e){
        Hexahedra& hex = hex_id();
        return hex.halfedge(edge2halfedge[e]);
    }

    HexModelerMode(){
        //import_new_project_from_CAD(std::string(TEST_INPUT_DIR) + "B1.geogram");
    }

    void load_project(){

        load_mm_with_default_layers(project_directory + "geom");
        load_mm_with_default_layers(project_directory + "mesh");

        Hexahedra& hex = hex_id();
        God::xcf["mesh"].polylines.clear();
        God::xcf["mesh"].polylines.try_emplace("");
        PolyLine& pl = pl_id();
        pl.points = hex.points;
        edge2halfedge.bind("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
        generate_edge_graph(hex, pl, edge2halfedge);

        PolyLineLayer& layer = God::layers.add<PolyLineLayer>("mesh_edge", 1);
        layer.init(pl_id());
        auto& pr = layer.primitive_renderer;        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;
        layer_selection_react.init(hex_id(), pl_id(), pts_id());
        God::modes.switch_to("layer_split");
    }



    void update_hex(){
        generate_edge_graph(hex_id(), pl_id(), edge2halfedge);
        hex_id().broadcast(EventType::UPDATED);
        pl_id().broadcast(EventType::UPDATED);
        pts_id().broadcast(EventType::UPDATED);
    }

    void handle(Event event){
            send_instruction_to_react(event, camera_react);
            send_instruction_to_react(event, layer_selection_react);
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
            mm.save_to_path(project_directory + "geom");

        }

        {
            MultiMesh mm;
            mm.hexahedra.try_emplace("");
            CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);
            //bbox
            mm.pointset.create_points(8);
            Hexahedra& hex = mm.hexahedra[""].mesh;
            hex.create_cells(1);
            FOR(k, 2)FOR(j, 2)FOR(i, 2) mm.pointset[i + 2 * j + 4 * k] = bbox.min + vec3(i * bbox.size()[0], j * bbox.size()[1], k * bbox.size()[2]);
            FOR(v, 8) hex.vert(0, v) = v;

            mm.save_to_path(project_directory + "mesh");
        }

    }

    void define_gui() {
        ImGui::Begin("HexModelerMode", nullptr);
        if (ImGui::Button("Import From CAD")) ImGui::OpenPopup("CAD_file");
        if (FilePopup("CAD_file", std::filesystem::path(CAD_file).parent_path().string(), CAD_file, {".geogram",".meshb",".mesh"}))
            if (!CAD_file.empty()) {
                project_directory = working_directory + std::filesystem::path(CAD_file).stem().string() + "\\";
                std::filesystem::create_directory(project_directory);
                import_new_project_from_CAD(CAD_file);
                load_project();
            }

        if (ImGui::Button("Load project")) ImGui::OpenPopup("Load project");
        if (FilePopup("Load project", project_directory, project_directory, {"*"}))
            if (!project_directory.empty()){
                project_directory = project_directory + "\\";
                load_project();
            }        
        ImGui::End();
    }

};




struct LayerSplitMode : public HexModelerMode{

    MouseReactMultiSelector multi_selector_react_left;
    MouseReactMultiSelector multi_selector_react_right;


    void activate(){
        edge2halfedge.bind("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
    }

    LayerSplitMode() : HexModelerMode(){
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


    void handle(Event event) {
        // forward mouse events to mouse reacts 
        if (God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
            HexModelerMode::handle(event);
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





    void define_gui() {
        ImGui::Begin("Mode", nullptr);
        ImGui::Text("Window used to launch debug tests");
        multi_selector_react_left.feed_back_gui();
        multi_selector_react_right.feed_back_gui();
        ImGui::End();
    }
};
