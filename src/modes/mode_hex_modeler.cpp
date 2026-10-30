#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/mode_hex_modeler.h>
#include <modes/submode_start.h>
#include <modes/submode_layer_split.h>
#include <modes/submode_feature_edit.h>




#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")


// access to xcf bjects
ObjectId HexModelerMode::hex_id(){ return ObjectId(ObjectType::HEXAHEDRA, std::vector<std::string>{ "mesh", ""}); }
ObjectId HexModelerMode::pl_id(){ return ObjectId(ObjectType::POLYLINES, std::vector<std::string>{ "mesh", ""}); };
ObjectId HexModelerMode::pts_id(){ return ObjectId(ObjectType::POINTSET, std::vector<std::string>{ "mesh"}); };
ObjectId HexModelerMode::geom_tri_id(){ return ObjectId(ObjectType::TRIANGLES, std::vector<std::string>{ "geom", ""}); }
ObjectId HexModelerMode::geom_edge_id(){ return ObjectId(ObjectType::POLYLINES, std::vector<std::string>{ "geom", ""}); }
ObjectId HexModelerMode::geom_pts_id(){ return ObjectId(ObjectType::POINTSET, std::vector<std::string>{ "geom"}); }


// update coherence constraints
Volume::Halfedge HexModelerMode::hex_halfedge_from_edge(int e){
    EdgeAttribute<int> edge2halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
    Hexahedra& hex = hex_id();
    return hex.halfedge(edge2halfedge[e]);
}
void HexModelerMode::update_hex(){
    EdgeAttribute<int> edge2halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);

    generate_edge_graph(hex_id(), pl_id(), edge2halfedge);
    hex_id().broadcast(EventType::UPDATED);
    pl_id().broadcast(EventType::UPDATED);
    pts_id().broadcast(EventType::UPDATED);
}

// manage camera + layer selection
void HexModelerMode::handle_camera_and_layerselect(Event event){
    send_instruction_to_react(event, camera_react);
    send_instruction_to_react(event, layer_selection_react);
}


// manage import/load/write of project
void HexModelerMode::reset_xcf(){
    std::vector<std::string> mms;
    for (auto& [key, value] : God::xcf) mms.push_back(key);
    for (auto it : mms) God::xcf.kill_multimesh(it);
    God::events.dispatch();
}
void HexModelerMode::load_project(){
    reset_xcf();
    load_mm_with_default_layers(project_directory + "geom");
    load_mm_with_default_layers(project_directory + "mesh");

    Hexahedra& hex = hex_id();
    God::xcf["mesh"].polylines.clear();
    God::xcf["mesh"].polylines.try_emplace("");
    PolyLine& pl = pl_id();
    pl.points = hex.points;

    EdgeAttribute<int> edge2halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
    generate_edge_graph(hex, pl, edge2halfedge);

    PolyLineLayer& layer = God::layers.add<PolyLineLayer>("meshEdges", 1);
    layer.init(pl_id());
    auto& pr = layer.edge_renderer;        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;
    layer_selection_react.init(hex_id(), pl_id(), pts_id());

    set_active_mode("LayerSplitMode");
}
void HexModelerMode::import_new_project_from_CAD(std::string filename){
    reset_xcf();
    BBox3 bbox;
    {   // import the CAD
        MultiMesh mm;
        mm.load_geogram(filename, false);
        FOR(v, mm.pointset.size()) bbox.add(mm.pointset[v]);

        if (mm.polylines.empty()) mm.polylines.try_emplace("");
        auto& pl = mm.polylines[""];
        pl.attributes.points.clear();


        std::vector<int> gmsh_import_feature(pl.mesh.nedges());
        {
            EdgeAttribute<int> region("region", pl.attributes, pl.mesh);
            for (auto e : pl.mesh.iter_edges()) gmsh_import_feature[e] = region[e];
        }
        pl.attributes.edges.clear();
        EdgeAttribute<int> feat_id("feat_id", pl.attributes, pl.mesh);
        for (auto e : pl.mesh.iter_edges()) feat_id[e] = gmsh_import_feature[e];

        {// create opposite edges
            pl.mesh.disconnect();
            int nedges = pl.mesh.nedges();
            pl.mesh.create_edges(pl.mesh.nedges());
            FOR(e, nedges){
                FOR(lv, 2)pl.mesh.vert(e + nedges, lv) = pl.mesh.vert(e, 1 - lv);
                feat_id[e + nedges] = feat_id[e];
            }
        }


        if (mm.triangles.empty()) mm.triangles.try_emplace("");
        auto& tri = mm.triangles[""];
        tri.attributes.points.clear();
        tri.attributes.corners.clear();
        tri.attributes.facets.clear();
        FacetAttribute<int> region("region_id", tri.attributes, tri.mesh);
        for (auto f : tri.mesh.iter_facets()) region[f] = f;

        mm.quads.clear(); mm.polygons.clear(); mm.hexahedra.clear(); mm.pyramids.clear(); mm.wedges.clear();
        mm.save_to_path(project_directory + "geom",true);

    }

    { // create an empty mesh
        MultiMesh mm;
        mm.hexahedra.try_emplace("");
        CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);
        
        mm.pointset.create_points(8);
        Hexahedra& hex = mm.hexahedra[""].mesh;
        hex.create_cells(1);
        FOR(k, 2)FOR(j, 2)FOR(i, 2) mm.pointset[i + 2 * j + 4 * k] = bbox.min + vec3(i * bbox.size()[0], j * bbox.size()[1], k * bbox.size()[2]);
        FOR(v, 8) hex.vert(0, v) = v;

        mm.save_to_path(project_directory + "mesh", true);
    }
    reset_xcf();
}


void HexModelerMode::set_active_mode(std::string s){
    int id = modes.find(s);
    um_assert(id != -1);
    active_mode_id = id;
    modes[id].activate();
}
void HexModelerMode::define_gui(){
    modes[active_mode_id].define_gui();
}
void HexModelerMode::handle(Event event) {
    modes[active_mode_id].handle(event);
};




HexModelerMode::HexModelerMode(){
    working_directory = WORKING_DIR;
    project_directory = WORKING_DIR;
    CAD_file = std::string(TEST_INPUT_DIR) + "B1.geogram";

    modes.emplace_back<StartingMode>("StartingMode", *this);
    modes.emplace_back<LayerSplitMode>("LayerSplitMode", *this);
    modes.emplace_back<FeatureEditMode>("FeatureEditMode", *this);

    if (1){
        project_directory = WORKING_DIR + "B1\\";
        load_project();
        set_active_mode("FeatureEditMode");
    }else
    set_active_mode("StartingMode");
}
