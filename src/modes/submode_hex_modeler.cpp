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
#include <modes/submode_basic_edit.h>



//#define WORKING_DIR std::string("/home/ssloy/+/hexel/build/NICO/")
#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")



HexModelerSubMode::HexModelerSubMode(HexModelerMode& mode) :layer_selection_react(*this), mode(mode){}

// access to xcf bjects
MeshPath HexModelerSubMode::hex_id(){ return root().xcf["mesh"].hexahedra[""]; }
MeshPath HexModelerSubMode::pl_id(){ return  root().xcf["mesh"].polylines[""]; };
PointSetPath HexModelerSubMode::pts_id(){ return root().xcf["mesh"].pointset; };
MeshPath HexModelerSubMode::geom_tri_id(){ return  root().xcf["geom"].triangles[""]; }
MeshPath HexModelerSubMode::geom_edge_id(){ return  root().xcf["geom"].polylines[""]; }
PointSetPath HexModelerSubMode::geom_pts_id(){ return  root().xcf["geom"].pointset; }


// update coherence constraints
Volume::Halfedge HexModelerSubMode::hex_halfedge_from_edge(int e){
    EdgeAttribute<int> edge2halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
    Hexahedra& hex = hex_id();
    return hex.halfedge(edge2halfedge[e]);
}
void HexModelerSubMode::update_hex(){
    Hexahedra& hex = hex_id();
    plop(hex.nverts());
    EdgeAttribute<int> edge2halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
    generate_edge_graph(hex_id(), pl_id(), edge2halfedge,false);

    VolumeAttributes& hex_attr = hex_id();
    CellAttribute<bool> visible_cell("visible", hex_attr, hex);

    PointSetAttributes& pts_attr = pts_id();
    PointSet& pts = pts_id();
    PointAttribute<bool> visible_pts("visible", pts_attr, pts);
    visible_pts.fill(true);
    for (auto c : hex.iter_cells()) if (!visible_cell[c])
        FOR(lv, 8)visible_pts[c.vertex(lv)] = false;

    for (auto c : hex.iter_cells()) if (visible_cell[c])
        FOR(lv, 8)visible_pts[c.vertex(lv)] = true;


    PolyLineAttributes& pl_attr = pl_id();
    PolyLine& pl = pl_id();
    EdgeAttribute<bool> visible_edge("visible", pl_attr, pl);
    visible_edge.fill(false);
    for (auto e : pl.iter_edges()) if (visible_pts[e.from()] && visible_pts[e.to()])
        visible_edge[e] = true;


    hex_id().broadcast(EventType::UPDATED);
    pl_id().broadcast(EventType::UPDATED);
    pts_id().broadcast(EventType::UPDATED);
}

// manage camera + layer selection
void HexModelerSubMode::handle_camera_and_layerselect(Event event){
    camera_react.handle(event);
    layer_selection_react.handle(event);
}


void HexModelerSubMode::backup(){
    int undo_stack_size = 5;
    mode.undo_id++;
    if (mode.undo_begin < mode.undo_id - mode.undo_stack_size){
        mode.undo_begin++;
        std::string kill_dir = WORKING_DIR + "undo" + std::to_string(mode.undo_begin);
        std::filesystem::remove_all(kill_dir);
    }

    std::string dir = WORKING_DIR + "undo" + std::to_string(mode.undo_id) + "/";
    std::filesystem::create_directory(dir);
    save_project(dir);
}
void HexModelerSubMode::undo(){
    if (mode.undo_id == mode.undo_begin) {
        Log::add("Undo stack is empty");
        return;
    }
    std::string dir = WORKING_DIR + "undo" + std::to_string(mode.undo_id);

    std::vector<std::string> mms;
    for (auto& [key, value] : God::xcf) mms.push_back(key);
    for (auto it : mms) God::xcf.kill_multimesh(it);
    God::events.queue.clear();

    reset_xcf();
    God::xcf.load_multimesh(dir + "/geom", true);
    God::xcf.load_multimesh(dir + "/mesh", true);
    update_hex();
    mode.undo_id--;
}




bool HexModelerSubMode::xcf_is_valid_for_editing(){
    if (!God::xcf.contains("mesh")) goto reset;
    if (!God::xcf.contains("geom")) goto reset;
    return true;
reset:
    reset_xcf();
    return false;
}

void HexModelerSubMode::reset_layers_options(){
    if (!xcf_is_valid_for_editing()) return;

    {// rendering of the geom object
        God::layers["geomPts"].visible = false;
        God::layers["geomTri"].visible = true;
        God::layers["geomEdges"].visible = true;


        static_cast<PointSetLayer&>(God::layers["geomPts"]).point_renderer.radius_in_pixel = 5;

        if (!God::layers.contains("geomLock")){
            PointSetLayer& layer = God::layers.add<PointSetLayer>("geomLock", 1);
            layer.group = 0;
            layer.init(geom_pts_id());
        }

        PointAttribute<int> touch_lock("lock", God::xcf["geom"].pointset_attributes, God::xcf["geom"].pointset, 0);
        God::layers["geomLock"].bind_attribute(ObjectId(ObjectType::POINTSET_ATTR, { "geom", "int","points","lock" }));
        {
            auto& layer = static_cast<PointSetLayer&>(God::layers["geomLock"]);
            layer.point_renderer.radius_in_pixel = 7;
            layer.skip_value = 0;
            layer.mask = true;
        }

        God::layers["geomEdges"].bind_attribute(ObjectId(ObjectType::POLYLINES_ATTR, { "geom", "","int","edges","feat_id" }));
        auto& er = static_cast<PolyLineLayer&>(God::layers["geomEdges"]).edge_renderer;
    }

    {// rendering of the hexahedral mesh
        if (!God::layers.contains("meshEdges")){
            PolyLineLayer& layer = God::layers.add<PolyLineLayer>("meshEdges", 1);
            layer.init(pl_id());
        }
        God::layers["meshPts"].visible = true;
        God::layers["meshHex"].visible = true;
        God::layers["meshEdges"].visible = true;

        PointAttribute<int> touch_lock("lock", God::xcf["mesh"].pointset_attributes, God::xcf["mesh"].pointset, 0);
        God::layers["meshPts"].bind_attribute(ObjectId(ObjectType::POINTSET_ATTR, { "mesh", "int","points","lock" }));
        {
            auto& layer = static_cast<PointSetLayer&>(God::layers["meshPts"]);
            layer.point_renderer.radius_in_pixel = 10;
        }

        God::layers["meshHex"].bind_attribute(ObjectId(ObjectType::HEXAHEDRA_ATTR, { "mesh", "","int","cells","block_id" }));
        //auto& pr = dynamic_cast<PolyLineLayer&> (God::layers["meshEdges"]).edge_renderer;        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;
    }

}


// manage import/load/write of project
void HexModelerSubMode::reset_xcf(){
    std::vector<std::string> mms;
    for (auto& [key, value] : God::xcf) mms.push_back(key);
    for (auto it : mms) God::xcf.kill_multimesh(it);
    God::events.dispatch();
}
void HexModelerSubMode::save_project(std::string directory){
    MultiMesh& geommm = ObjectId(MULTIMESH, "geom");
    geommm.save_to_path(directory + "geom", true);
    MultiMesh& meshmm = ObjectId(MULTIMESH, "mesh");
    meshmm.save_to_path(directory + "mesh", true);

}

void HexModelerSubMode::load_project(std::string directory){
    reset_xcf();
    const std::filesystem::path project_dir{directory};
    load_mm_with_default_layers((project_dir / "geom").string());
    load_mm_with_default_layers((project_dir / "mesh").string());

    Hexahedra& hex = hex_id();                                       
    God::xcf["mesh"].polylines.clear();
    God::xcf["mesh"].polylines.try_emplace("");
    PolyLine& pl = pl_id();
    pl.points = hex.points;

    EdgeAttribute<int> edge2halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
    generate_edge_graph(hex, pl, edge2halfedge);

    reset_layers_options();
    //layer_selection_react.init(hex_id(), pl_id(), pts_id());

    //set_active_mode("BasicEditMode");
}

