#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/X/root_mode.h>
#include <modes/X/mode_start.h>
#include <modes/X/mode_layer_split.h>
#include <modes/X/mode_feature.h>
#include <modes/X/cmd.h>
#include <modes/X/mode_basic.h>
#include <modes/X/scene.h>
#include <modes/X/inspect.h>
//


namespace ModeX{

    // access to xcf objects
    MeshPath XCFview::hex_id(){ return root.xcf["mesh"].hexahedra[""]; }
    MeshPath XCFview::pl_id(){ return  root.xcf["mesh"].polylines[""]; };
    PointSetPath XCFview::pts_id(){ return root.xcf["mesh"].pointset; };
    MeshPath XCFview::geom_tri_id(){ return  root.xcf["geom"].triangles[""]; }
    MeshPath XCFview::geom_edge_id(){ return  root.xcf["geom"].polylines[""]; }
    PointSetPath XCFview::geom_pts_id(){ return  root.xcf["geom"].pointset; }




    bool Scene::update_hex(){
        Hexahedra& hex = hex_id();
        PolyLine& pl = pl_id();
        pl.disconnect();
        hex.disconnect();
        {//clean_up
            std::vector<bool> to_kill(pl.nedges(), true);
            pl.delete_edges(to_kill);
            hex.delete_isolated_vertices();
        }
        {//generate_edge_graph
            if (!hex.connected()) hex.connect();
            DisjointSet ds(hex.nhalfedges());
            for (auto h : hex.iter_halfedges())
                if (h.facet().opposite().active())
                    ds.merge(h, h.opposite_c().opposite_f());

            std::vector<int> edges_;
            int nedges = ds.get_sets_id(edges_);
            pl.create_edges(nedges);
            for (auto h : hex.iter_halfedges()) {
                int e = edges_[h];
                pl.vert(e, 0) = h.from();
                pl.vert(e, 1) = h.to();
            }
            pl.connect();
        }




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
        EdgeAttribute<bool> visible_edge("visible", pl_attr, pl);
        visible_edge.fill(false);
        for (auto e : pl.iter_edges()) if (visible_pts[e.from()] && visible_pts[e.to()])
            visible_edge[e] = true;

        PointAttribute<int> emb_lock("emb_lock", God::xcf["mesh"].pointset_attributes, God::xcf["mesh"].pointset, 0);
        FOR(v, pts.size()){
            auto emb = Inspect(mode).get_embedding(pts[v]);
            if (emb_lock[v] != 5)
                emb_lock[v] = emb.dim;
        }

        hex_id().broadcast(Event::UPDATED);
        pl_id().broadcast(Event::UPDATED);
        pts_id().broadcast(Event::UPDATED);
        return true;
    }


    void Scene::backup(){
        int undo_stack_size = 5;
        mode.undo_id++;
        if (mode.undo_begin < mode.undo_id - mode.undo_stack_size){
            mode.undo_begin++;
            std::string kill_dir = mode.working_directory + "undo" + std::to_string(mode.undo_begin);
            std::filesystem::remove_all(kill_dir);
        }

        std::string dir = mode.working_directory + "undo" + std::to_string(mode.undo_id) + "/";
        std::filesystem::create_directory(dir);
        save_project(dir);
    }

    void Scene::forget_last_backup(){
        if (mode.undo_id == mode.undo_begin) {
            Log::add("Undo stack is empty, not able to forget");
            return;
        }
        mode.undo_id--;
    }
    void Scene::undo(){
        if (mode.undo_id == mode.undo_begin) {
            Log::add("Undo stack is empty");
            return;                                                                           
        }
        std::string dir = mode.working_directory + "undo" + std::to_string(mode.undo_id);

        std::vector<std::string> mms;
        for (auto& [key, value] : God::xcf) mms.push_back(key);
        for (auto it : mms) God::xcf.kill_multimesh(it);
        God::events.queue.clear();
        root.xcf["panic"].pointset.broadcast(Event::KILLED);
        God::events.dispatch();

        reset_xcf();
        God::xcf.load_multimesh(dir + "/geom", true);
        God::xcf.load_multimesh(dir + "/mesh", true);
        mode.undo_id--;
        Commands(mode.sub_mode(),false);
    }

    bool Scene::xcf_is_valid_for_editing(){
        if (!God::xcf.contains("mesh")) goto reset;
        if (!God::xcf.contains("geom")) goto reset;
        return true;
    reset:
        reset_xcf();
        return false;
    }

    void Scene::restore_default_scene_rendering(){
        if (!xcf_is_valid_for_editing()) return;

        {// rendering of the geom object

            {// geom points are invisible by default to make it easier to pick triangles
                God::layers["geomPts"].visible = false;
                static_cast<PointSetLayer&>(God::layers["geomPts"]).point_renderer.radius_in_pixel = 5;
            }
            {// geom triangles are rendered as plain
                God::layers["geomTri"].visible = true;
            }

            {// geom features are rendered by ids 
                God::layers["geomEdges"].visible = true;
                God::layers["geomEdges"].bind_attribute(root.xcf["geom"].polylines[""].attr_edges["feat_id"]);
                auto& er = static_cast<PolyLineLayer&>(God::layers["geomEdges"]).edge_renderer;
            }

            {// geomLock is created if needed, bound to locked vertices, and masked to show only locked vertices 
                if (!God::layers.contains("geomLock")){
                    PointSetLayer& layer = God::layers.add<PointSetLayer>("geomLock", God::layers.find_group_ptr("geom")->id);
                    layer.group = 0;
                    layer.init(root.xcf["mesh"].pointset);
                }

                PointAttribute<int> touch_lock("lock", God::xcf["geom"].pointset_attributes, God::xcf["geom"].pointset, 0);
                God::layers["geomLock"].bind_attribute(root.xcf["geom"].pointset.attr_points["lock"]);

                {
                    auto& layer = static_cast<PointSetLayer&>(God::layers["geomLock"]);
                    layer.point_renderer.radius_in_pixel = 10;
                    layer.skip_value = 0;
                    layer.mask = true;
                    layer.update();
                }
            }
        }

        {// rendering of the hexahedral mesh
            {// hex points are shown with their lock attribute
                God::layers["meshPts"].visible = true;
                PointAttribute<int> emb_lock("emb_lock", God::xcf["mesh"].pointset_attributes, God::xcf["mesh"].pointset, God::layers.find_group_ptr("geom")->id);
                God::layers["meshPts"].bind_attribute(root.xcf["mesh"].pointset.attr_points["emb_lock"]);
                {
                    auto& layer = static_cast<PointSetLayer&>(God::layers["meshPts"]);
                    layer.point_renderer.radius_in_pixel = 10;
                    glDeleteTextures(1, &layer.point_renderer.colormap_texture);
                    layer.point_renderer.colormap = Colormap::RAINBOW;
                    layer.point_renderer.auto_range = false;
                    layer.point_renderer.texture_range[0] = -1;
                    layer.point_renderer.texture_range[1] = 5;
                    load_colormap(layer.point_renderer.colormap, layer.point_renderer.colormap_texture);
                }
            }

            {// hexahedra arr shown with their block id
                God::layers["meshHex"].visible = true;
                God::layers["meshHex"].bind_attribute(root.xcf["mesh"].hexahedra[""].attr_cells["block_id"]);
                HexahedraLayer& layer = static_cast<HexahedraLayer&>(God::layers["meshHex"]);
                glDeleteTextures(1, &layer.tri_renderer.colormap_texture);
                layer.tri_renderer.colormap = Colormap::COOLWARM;
                load_colormap(layer.tri_renderer.colormap, layer.tri_renderer.colormap_texture);
            }

            {// mesh edges are rendered with plain style
                if (!God::layers.contains("meshEdges")){
                    PolyLineLayer& layer = God::layers.add<PolyLineLayer>("meshEdges", God::layers.find_group_ptr("mesh")->id);
                    layer.init(root.xcf["mesh"].polylines[""]);
                }
                God::layers["meshEdges"].visible = true;
            }
        }
        
        for (auto& group : God::layers.groups)
            if (group.name == "mesh")
                group.blend_op = Group::BlendOp::AVERAGE;
    }

    // manage import/load/write of project
    void Scene::reset_xcf(){
        std::vector<std::string> mms;
        for (auto& [key, value] : God::xcf) mms.push_back(key);
        for (auto it : mms) God::xcf.kill_multimesh(it);
        God::events.dispatch();
    }
    void Scene::save_project(std::string directory){
        MultiMesh& geommm = God::xcf["geom"];
        geommm.save_to_path(directory + "geom", true);
        MultiMesh& meshmm = God::xcf["mesh"];
        meshmm.save_to_path(directory + "mesh", true);
    }
    void Scene::load_project(std::string directory){
        reset_xcf();
        const std::filesystem::path project_dir{ directory };
        load_mm_with_default_layers((project_dir / "geom").string());
        load_mm_with_default_layers((project_dir / "mesh").string());

        Hexahedra& hex = God::xcf["mesh"].hexahedra[""].mesh;
        God::xcf["mesh"].polylines.clear();
        God::xcf["mesh"].polylines.try_emplace("");
        PolyLine& pl = God::xcf["mesh"].polylines[""].mesh;
        pl.points = hex.points;

        //EdgeAttribute<int> edge2halfedge("hex_halfedge", God::xcf["mesh"].polylines[""].attributes, God::xcf["mesh"].polylines[""].mesh);
        //generate_edge_graph(hex, pl);
        update_hex();
        restore_default_scene_rendering();
    }
};