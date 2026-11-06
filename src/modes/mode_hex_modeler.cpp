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

    Hexahedra& hex = hex_id();
    VolumeAttributes& hex_attr = hex_id();
    CellAttribute<bool> visible_cell("visible", hex_attr, hex);

    PointSetAttributes& pts_attr = pts_id();
    PointSet& pts = pts_id();
    PointAttribute<bool> visible_pts("visible", pts_attr, pts);
    visible_pts.fill(false);
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
void HexModelerMode::handle_camera_and_layerselect(Event event){
    send_instruction_to_react(event, camera_react);
    send_instruction_to_react(event, layer_selection_react);
}


bool HexModelerMode::xcf_is_valid_for_editing(){
    if (!God::xcf.contains("mesh")) goto reset;
    if (!God::xcf.contains("geom")) goto reset;
    return true;
reset:
    reset_xcf();
    return false;
}

void HexModelerMode::reset_layers_options(){
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

        auto& pr = dynamic_cast<PolyLineLayer&> (God::layers["meshEdges"]).edge_renderer;        pr.color[0] = .5; pr.color[1] = .5; pr.color[2] = .7;
    }





}


// manage import/load/write of project
void HexModelerMode::reset_xcf(){
    std::vector<std::string> mms;
    for (auto& [key, value] : God::xcf) mms.push_back(key);
    for (auto it : mms) God::xcf.kill_multimesh(it);
    God::events.dispatch();
}
void HexModelerMode::save_project(std::string directory){
    MultiMesh& geommm = ObjectId(MULTIMESH, "geom");
    geommm.save_to_path(directory + "geom", true);
    MultiMesh& meshmm = ObjectId(MULTIMESH, "mesh");
    meshmm.save_to_path(directory + "mesh", true);

}

void HexModelerMode::load_project(std::string directory){
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
    layer_selection_react.init(hex_id(), pl_id(), pts_id());

    set_active_mode("BasicEditMode");
}
void HexModelerMode::import_new_project_from_CAD(std::string filename){
    const std::filesystem::path project_dir{project_directory};
    std::filesystem::create_directory(project_dir);

    BBox3 bbox;
    {   // import the CAD
        MultiMesh mm;
        mm.load_geogram(filename, false);
        FOR(v, mm.pointset.size()) bbox.add(mm.pointset[v]);

        PointAttribute<int> touch_lock("lock", mm.pointset_attributes, mm.pointset, 0);


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
        pl.mesh.connect();
        for (auto v : pl.mesh.iter_vertices()) for (auto e0 : v.iter_edges())for (auto e1 : v.iter_edges())
            if (feat_id[e0] != feat_id[e1]) touch_lock[v] = 1;


        if (mm.triangles.empty()) mm.triangles.try_emplace("");
        auto& tri = mm.triangles[""];
        tri.attributes.points.clear();
        tri.attributes.corners.clear();
        tri.attributes.facets.clear();
        FacetAttribute<int> region("region_id", tri.attributes, tri.mesh);
        for (auto f : tri.mesh.iter_facets()) region[f] = f;

        mm.quads.clear(); mm.polygons.clear(); mm.hexahedra.clear(); mm.pyramids.clear(); mm.wedges.clear();
        mm.save_to_path((project_dir / "geom").string(),true);
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

        mm.save_to_path((project_dir / "mesh").string(), true);
    }
}

void HexModelerMode::backup(){
    int undo_stack_size = 5;
    undo_id++;
    if (undo_begin < undo_id - undo_stack_size){
        undo_begin++; 
        std::string kill_dir = WORKING_DIR + "undo" + std::to_string(undo_begin) ;
        std::filesystem::remove_all(kill_dir);
    }

    std::string dir = WORKING_DIR + "undo" + std::to_string(undo_id)+"/";
    std::filesystem::create_directory(dir);
    save_project(dir);
}
void HexModelerMode::undo(){
    if (undo_id == undo_begin) {
        Log::add("Undo stack is empty");
        return;
    }
    std::string dir = WORKING_DIR + "undo" + std::to_string(undo_id);

    std::vector<std::string> mms;
    for (auto& [key, value] : God::xcf) mms.push_back(key);
    for (auto it : mms) God::xcf.kill_multimesh(it);
    God::events.queue.clear();

    reset_xcf();
    God::xcf.load_multimesh(dir + "/geom", true);
    God::xcf.load_multimesh(dir + "/mesh", true);
    update_hex();                                                        
    undo_id--;
}

void HexModelerMode::reset_gui(){
    load_project(project_directory);
}

void HexModelerMode::set_active_mode(std::string s){
    int id = modes.find(s);
    um_assert(id != -1);
    active_mode_id = id;
    reset_layers_options();
    modes[id].activate();
}

static void ImGui_help(const char* desc){
    ImGui::SameLine(); 
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
void HexModelerMode::header_gui_for_submodes(){
    if (ImGui::Button("Backup")) backup();
    ImGui::SameLine();
    if (ImGui::Button("Undo")) undo();
    ImGui::SameLine();
    if (ImGui::Button("Reset GUI")) reset_gui();
    ImGui::Separator();

    const char* items[] = { "Load/Import", "BasicEdit", "LayerSplit", "CAD_feature" };
    static int current_mode = 1;

    if (ImGui::Combo("SubMode", &current_mode, items, IM_COUNTOF(items))){
        if (current_mode == 0) set_active_mode("StartingMode");
        if (current_mode == 1) set_active_mode("BasicEditMode");
        if (current_mode == 2) set_active_mode("LayerSplitMode");
        if (current_mode == 3) set_active_mode("FeatureEditMode");
    }
    
    ImGui_help("Choose the current submode\n" "hopefully you can go there");
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
    undo_stack_size = 5;

    modes.emplace_back<StartingMode>("StartingMode", *this);
    modes.emplace_back<BasicEditMode>("BasicEditMode", *this);
    modes.emplace_back<LayerSplitMode>("LayerSplitMode", *this);
    modes.emplace_back<FeatureEditMode>("FeatureEditMode", *this);

    if (1){
        project_directory = WORKING_DIR + "B1";
        import_new_project_from_CAD(CAD_file);
        load_project(project_directory);
        set_active_mode("BasicEditMode");
    }else
        set_active_mode("StartingMode");
}
