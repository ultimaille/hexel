#pragma once
#include <modes/X/feedback_3d.h>
#include <modes/X/abstract_mode.h>

namespace ModeX{

    ENUM(QuadToHexGeomStrategy, OFFSET_PROP_TO_QUAD, RAY_TRACE, FRAME_FIELD);
    ENUM(QuadToHexTopoStrategy, AUTO_MERGE, NO_MERGE);


    // algorithms that are ready to call from UI
    struct Commands {

        Commands(AbstractMode& mode, bool backup = true);
        static FeedBack3D feedback;

        ~Commands();
        AbstractMode& mode;

        void update_panic_in_xcf(std::vector<int>& conflicts_topo, std::vector<int>& conflicts_geom);


        void merge_vertices(int source, int target);
        void move_vertex(int moving_vertex, vec3 new_pos);

        //enum Quad2HexGeomStrategy { OFFSET_BY_QUAD_NORMAL, RAYTRACE };
        enum Quad2HexTopoStrategy { AUTO_MERGE, NO_MERGE };
        void create_hex_from_quad(std::vector<vec3>& pts, QuadToHexGeomStrategy geom_strat, QuadToHexTopoStrategy topo_strat);
        void extend_mesh_from_face(int f_id);

        void kill_hexes(std::vector<bool>& to_kill);

    };
};

//#include <modes/X/toolbox.h>
//namespace ModeX{
//
//    ENUM(QuadToHexGeomStrategy, OFFSET_PROP_TO_QUAD, RAY_TRACE, FRAME_FIELD);
//
//
//    // algorithms that are ready to call from UI
//    struct Commands {
//
//        Commands(AbstractMode& mode,bool backup=true) :mode(mode) {
//            if (backup) mode.scene().backup();
//        }
//        static FeedBack3D feedback;
//
//
//        ~Commands() {
//            mode.scene().update_hex();
//            std::vector<int> conflicts_topo;
//            std::vector<int> conflicts_geom;
//            mode.mode.hex_state = mode.mode.ALL_OK;
//            if (!ToolBoxHexahedra(mode.hex_id()).validate_topo(conflicts_topo))
//                mode.mode.hex_state = mode.mode.hex_state - RootMode::TOPO_OK;
//            if (!ToolBoxHexahedra(mode.hex_id()).validate_geom(conflicts_geom))
//                mode.mode.hex_state = mode.mode.hex_state - RootMode::GEOM_OK;
//            update_panic_in_xcf(conflicts_topo, conflicts_geom);
//            if (mode.mode.hex_state != RootMode::ALL_OK) {
//                Log::info("Hex state is broken");
//                return;
//            }
//            mode.hex_id().broadcast(Event::UPDATED);
//        }
//        AbstractMode& mode;
//
//        void update_panic_in_xcf(std::vector<int>& conflicts_topo, std::vector<int>& conflicts_geom){
//            if (conflicts_topo.empty() && conflicts_geom.empty()){
//                if (feedback.activated()) feedback.desactivate();
//                return;
//            }
//            Hexahedra& hex = mode.hex_id();
//            if (!feedback.activated()) feedback.activate();
//            auto& layer = static_cast<PointSetLayer&>(God::layers[feedback.name + "Pts"]);
//            layer.point_renderer.radius_in_pixel = 15;
//            FOR(d, 3) layer.point_renderer.color[d] = d == 0 ? 255 : 0;
//
//
//            feedback.begin_mesh_update();
//            for (auto &set : { conflicts_topo ,conflicts_geom }){
//                std::vector<vec3> pts(set.size());
//                FOR(i, pts.size())pts[i] = hex.points[set[i]];
//                feedback.add_points(pts);
//            }
//            feedback.end_mesh_update();
//        }
//
//
//        void merge_vertices(int source, int target) {
//            Hexahedra& hex = mode.hex_id();
//            ToolBox(hex).merge_vertices(source, target);
//        }
//        void move_vertex(int moving_vertex, vec3 new_pos){
//            PointSet& pts = mode.pts_id();
//            pts[moving_vertex] = new_pos;
//        }
//
//        enum Quad2HexGeomStrategy { OFFSET_BY_QUAD_NORMAL, RAYTRACE };
//        enum Quad2HexTopoStrategy { AUTO_MERGE, NO_MERGE };
//        void create_hex_from_quad(std::vector<vec3>& pts, Quad2HexGeomStrategy geom_strat, Quad2HexTopoStrategy topo_strat) {
//            MultiMesh& mm = God::xcf["mesh"];
//            Hexahedra& hex = mode.hex_id();
//            CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);
//
//            vec3 hex_pts[8] = {
//                pts[0], pts[1], pts[3], pts[2],
//                pts[0], pts[1], pts[3], pts[2]
//            };
//
//            if (geom_strat == OFFSET_BY_QUAD_NORMAL){
//                Quad3 quad(pts[0], pts[1], pts[2], pts[3]);
//                vec3 decal = -.2 * std::sqrt(quad.unsigned_area()) * quad.normal();
//                FOR(lv, 4) hex_pts[lv] += decal;
//            }
//            else if (geom_strat == RAYTRACE){
//                Quad3 quad(pts[0], pts[1], pts[2], pts[3]);
//                //RayTracer ray_trace(mode.geom_tri_id());
//                vec3 n = -quad.normal();
//
//                double min_decal = .05 * std::sqrt(quad.unsigned_area());
//                Triangles& tri = mode.geom_tri_id();
//                FOR(lv, 4) {
//                    vec3 dir = n;
//                    Inspect::Embedding emb = mode.inspect().get_embedding(hex_pts[lv]);
//                    if (emb.dim == 2) {
//                        Surface::Facet f(tri, emb.id);
//                        dir = Triangle3(f).normal();
//                        if (dir * n < 0) dir = -dir;
//                    }
//                    hex_pts[lv] = ToolBox(tri).next_intersection(hex_pts[lv] + min_decal * dir, dir);
//                }
//            }
//            else{
//                Log::error("Strategy is not yet implemented, abort hex creation");
//                return;
//            }
//            int hex_vert[8];
//            if (topo_strat == NO_MERGE){
//                int offset = hex.points.create_points(8);
//                FOR(lv, 8) hex_vert[lv] = offset + lv;
//            }
//            else if (topo_strat == AUTO_MERGE){
//                // merge vertices clicked by the user
//                FOR(lv, 8) {
//                    hex_vert[lv] = -1;
//                    for (auto v : hex.iter_vertices())
//                        if (mode.inspect().on_point(hex_pts[lv], v.pos()))
//                            hex_vert[lv] = v;
//                    if (hex_vert[lv] == -1)
//                        hex_vert[lv] = hex.points.create_points(1);
//                }
//                // attempt to merge hex facets for an interface a la quad retopo 
//                EdgeGraph eg(hex);
//                int next_around_quad[4] = { 1,3,0,2 };
//                FOR(lh_org, 4){
//                    EdgeGraph::Vertex v(eg, hex_vert[lh_org + 4]);
//                    int lh_dest = next_around_quad[lh_org];
//                    for (auto e : v.iter_edges()){
//                        if (e.to() != hex_vert[lh_dest + 4]) continue;
//                        Volume::Halfedge h = eg.halfedge_from_edge(e);
//                        if (!h.active()) continue;
//                        if (h.facet().opposite().active()) continue;
//                        hex_vert[lh_org] = h.prev().from();
//                        hex_vert[lh_dest] = h.next().to();
//                    }
//                }
//            }
//            else{
//                Log::error("Strategy is not yet implemented, abort hex creation");
//                return;
//            }
//
//            int c = hex.create_cells(1);
//            block_id[c] = c;
//            FOR(lv, 8) hex.vert(c, lv) = hex_vert[lv];
//            FOR(lv, 8) hex.points[hex_vert[lv]] = hex_pts[lv];
//        }
//        void extend_mesh_from_face(int f_id){
//            MultiMesh& mm = God::xcf["mesh"];
//            Hexahedra& hex = mode.hex_id();
//            CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);
//
//            Volume::Facet f(hex, f_id);
//            // create an hex
//            vec3 n = Quad3(f).normal();
//            double scale = std::sqrt(Quad3(f).unsigned_area());
//            int hex_vert[8];
//            hex_vert[0] = f.vertex(0);
//            hex_vert[1] = f.vertex(1);
//            hex_vert[2] = f.vertex(3);
//            hex_vert[3] = f.vertex(2);
//            int offset = hex.points.create_points(4);
//            FOR(lv, 4) {
//                hex_vert[4 + lv] = offset + lv;
//                hex.points[offset + lv] = hex.points[hex_vert[lv]] + scale * n;
//            }
//
//            int new_cell = hex.create_cells(1);
//            block_id[new_cell] = new_cell;
//            FOR(lv, 8) hex.vert(new_cell, lv) = hex_vert[lv];
//            hex.connect();
//
//            // try to connect the other faces of the hex
//            bool no_more_possible_face_connection = false;
//            while (!no_more_possible_face_connection){
//                no_more_possible_face_connection = true;
//                Volume::Cell c(hex, new_cell);
//                for (auto f : c.iter_facets()){
//                    if (!f.opposite().active()) continue;
//                    for (auto h : f.iter_halfedges()){
//                        Volume::Halfedge it = h.opposite_f();
//                        if (it.facet().opposite().active()) continue;//  face is already connected
//                        int nn = 0;
//                        while (it.active()) {
//                            it = it.opposite_f().opposite_c();
//                            nn++;
//                        }
//                        if (nn == 4){
//                            no_more_possible_face_connection = false;
//                            it = h;
//                            FOR(i, 3) it = it.opposite_c().opposite_f();
//                            int to_merge[2][2] = {
//                                { h.opposite_f().next().to(),it.prev().from() },
//                                { h.opposite_f().prev().from(),it.next().to() }
//                            };
//                            FOR(i, 2) ToolBoxHexahedra(hex).merge_vertices(to_merge[i][0], to_merge[i][1]);
//                        }
//                    }
//                }
//                hex.connect();
//            }
//        }
//    
//        void kill_hexes(std::vector<bool>& to_kill){
//            Hexahedra& hex = mode.hex_id();
//            hex.disconnect();
//            hex.delete_cells(to_kill);
//        }
//
//};
//};