#include <modes/submode_basic_edit.h>
#include <modes/mode_hex_modeler.h>


// low-level (at UM level), without guaranties
namespace  AlgoRaw{
    void merge_vertices(Hexahedra& hex, int source, int target){
        FOR(c, hex.ncells())FOR(lv, 8) if (hex.vert(c, lv) == source)hex.vert(c, lv) = target;
    }

};

// Algorithms assumed to preserve some properties (returns bool if it doesn't) 
namespace AlgoMesh{
    bool merge_vertices(Hexahedra& hex, int source, int target){
        AlgoRaw::merge_vertices(hex, source, target);
        FOR(c, hex.ncells())FOR(lv0, 8) FOR(lv, 8) if (hex.vert(c, lv) == source)hex.vert(c, lv) = target;
        return true;
    }
};





struct RayTracer{
    RayTracer(Triangles& tri) : tri(tri){}
    vec3 next_intersection(vec3 O, vec3 dir){
        static const double infty = 1e20;
        double best_t = infty;
        for (auto f : tri.iter_facets()){
            Triangle3 tr(f);
            vec3 A = tr[0]; vec3 B = tr[1]; vec3 C = tr[2];
            double t = 1e20;
            if (intersectRayTriangle(O, dir, A, B, C, t))
                best_t = std::min(t, best_t);
        }
        if (best_t < infty)
            return O + best_t * dir;
        return O;
    }
private:
    bool intersectRayTriangle(const vec3& O, const vec3& dir, const vec3& A, const vec3& B, const vec3& C, double& t){
        const double EPSILON = 1e-9;
        vec3 AB = B - A;
        vec3 AC = C - A;
        vec3 h = cross(dir, AC);
        double det = AB * h;
        if (std::abs(det) < EPSILON) return false;
        double invDet = 1.0 / det;
        vec3 s = O - A;
        double u = invDet * (s * h);
        if (u < 0.0 || u > 1.0) return false;
        vec3 q = cross(s, AB);
        double v = invDet * (dir * q);
        if (v < 0.0 || u + v > 1.0) return false;
        t = invDet * (AC * q);
        if (t < EPSILON) return false;
        return true;
    }
    Triangles& tri;
};



// algorithms that are ready to call from UI
struct AlgoXCFLevel {
    AlgoXCFLevel(HexModelerSubMode& mode) :mode(mode) {}
    HexModelerSubMode& mode;
    bool merge_vertices(int source, int target) {
        mode.backup();
        Hexahedra& hex = mode.hex_id();
        if (!AlgoMesh::merge_vertices(hex, source, target)) {
            mode.undo();
            return false;
        }
        if (!mode.update_hex()) {
            mode.undo();
            return false;
        }
        mode.forget_last_backup();
        mode.hex_id().broadcast(Event::UPDATED);
        return true;
    }



    enum Quad2HexGeomStrategy {
        OFFSET_BY_QUAD_NORMAL,
        RAYTRACE
    };
    enum Quad2HexTopoStrategy {
        AUTO_MERGE,
        NO_MERGE
    };
    bool create_hex_from_quad(std::vector<vec3>& pts, Quad2HexGeomStrategy geom_strat, Quad2HexTopoStrategy topo_strat) {
        MultiMesh& mm = God::xcf["mesh"];
        Hexahedra& hex = mode.hex_id();
        CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);

        vec3 hex_pts[8] = {
            pts[0], pts[1], pts[3], pts[2],
            pts[0], pts[1], pts[3], pts[2]
        };

        if (geom_strat == OFFSET_BY_QUAD_NORMAL){
            Quad3 quad(pts[0], pts[1], pts[2], pts[3]);
            vec3 decal = -.2 * std::sqrt(quad.unsigned_area()) * quad.normal();
            FOR(lv, 4) hex_pts[lv] += decal;
        }
        else if (geom_strat == RAYTRACE){
            Quad3 quad(pts[0], pts[1], pts[2], pts[3]);
            RayTracer ray_trace(mode.geom_tri_id());
            vec3 n = -quad.normal();
            
            double min_decal = .05 * std::sqrt(quad.unsigned_area());
            Triangles& tri = mode.geom_tri_id();
            FOR(lv, 4) {
                vec3 dir = n;
                HexModelerSubMode::Embedding emb = mode.get_embedding(hex_pts[lv]);
                if (emb.dim == 2) {
                    Surface::Facet f(tri, emb.id);
                    dir = Triangle3(f).normal();
                    if (dir * n < 0) dir = -dir ;
                }
                hex_pts[lv] = ray_trace.next_intersection(hex_pts[lv] + min_decal * dir, dir);
            }
        }
        else{
            Log::error("Strategy is not yet implemented, abort hex creation");
            return false;
        }
        int hex_vert[8];
        if (topo_strat == NO_MERGE){
            int offset = hex.points.create_points(8);
            FOR(lv, 8) hex_vert[lv] = offset + lv;
        }
        else if (topo_strat == AUTO_MERGE){
            // merge vertices clicked by the user
            FOR(lv, 8) {
                hex_vert[lv] = -1;
                for (auto v : hex.iter_vertices())
                    if (mode.on_point(hex_pts[lv], v.pos()))
                        hex_vert[lv] = v;
                if (hex_vert[lv] == -1)
                    hex_vert[lv] = hex.points.create_points(1);
            }
            // attempt to merge hex facets for an interface a la quad retopo 
            EdgeGraph eg(hex);
            int next_around_quad[4] = { 1,3,0,2 };
            FOR(lh_org, 4){
                EdgeGraph::Vertex v(eg, hex_vert[lh_org + 4]);
                int lh_dest = next_around_quad[lh_org];
                for (auto e : v.iter_edges()){
                    if (e.to() != hex_vert[lh_dest + 4]) continue;
                    Volume::Halfedge h = eg.halfedge_from_edge(e);
                    if (!h.active()) continue;
                    if (h.facet().opposite().active()) continue;
                    hex_vert[lh_org] = h.prev().from();
                    hex_vert[lh_dest] = h.next().to();
                }
            }
        }
        else{
            Log::error("Strategy is not yet implemented, abort hex creation");
            return false;
        }

        int c = hex.create_cells(1);
        block_id[c] = c;
        FOR(lv, 8) hex.vert(c, lv) = hex_vert[lv];
        FOR(lv, 8) hex.points[hex_vert[lv]] = hex_pts[lv];
        mode.update_hex();
        mode.hex_id().broadcast(Event::UPDATED);
        return true;


        //int offset = hex.points.create_points(8);
        //FOR(lv, 4) hex.points[offset + lv] = pts[lv];
        //int c = hex.create_cells(1);
        //block_id[c] = c;
        //hex.vert(c, 0) = offset + 0;
        //hex.vert(c, 1) = offset + 1;
        //hex.vert(c, 2) = offset + 3;
        //hex.vert(c, 3) = offset + 2;
        //FOR(lv, 4) hex.vert(c, 4 + lv) = offset + 4 + lv;
        //FOR(lv, 4) hex.points[offset + 4 + lv] = hex.points[hex.vert(c, lv)] + decal;
        //mode.update_hex();
        //mode.hex_id().broadcast(Event::UPDATED);
        //return true;
    }





};



InteractorMoveVertex::InteractorMoveVertex(BasicEditMode& mode) :mode(mode){ moving_vertex = -1; }
void InteractorMoveVertex::on_press(int button, vec2 p) {
    if (!key_state_is_OK()) return;
    if (button != 0) return;
    if (mode.interactor_create_hex.in_progess()) return;
    picker.reset();
    for (auto& d : picker.data) {
        auto result = d.at(p);
        int picked_vert = result.filtered_primitive_id(mode.pts_id(), AttributeBase::TYPE::POINTS);
        if (picked_vert != -1) moving_vertex = picked_vert;
    }
}
void InteractorMoveVertex::on_release(int button, vec2 p) {
    if (!key_state_is_OK()) return;
    if (button != 0) return;
    if (moving_vertex == -1) return;
    PointSet& pts = God::xcf["mesh"].pointset;
    picker.reset();

    {// try to merge the vertex with another one
        int target_vertex = -1;
        for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_vert = result.filtered_primitive_id(mode.pts_id(), AttributeBase::TYPE::POINTS);
            if (picked_vert != -1) target_vertex = picked_vert;
        }
        if (target_vertex != -1) {
            Log::add("need to merge vertices");
            AlgoXCFLevel(mode).merge_vertices(moving_vertex, target_vertex);
            reset();
            return;
        }
    }
    bool new_position_is_found = false;;
    {// try to move the vertex on the geometry
        for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_triangle = result.filtered_primitive_id(mode.geom_tri_id(), AttributeBase::TYPE::FACETS);
            if (picked_triangle != -1) {
                Log::add("need to move vertex to picked triangle");
                Triangles& tri = mode.geom_tri_id();
                Surface::Facet f(tri, picked_triangle);
                pts[moving_vertex] = Triangle3(f).nearest_point(result.point());
                new_position_is_found = true;
            }
        }
        if (!new_position_is_found) for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_edge = result.filtered_primitive_id(mode.geom_edge_id(), AttributeBase::TYPE::EDGES);
            if (picked_edge != -1) {
                Log::add("need to move vertex to picked edge");
                PolyLine& pl = mode.geom_edge_id();
                PolyLine::Edge e(pl, picked_edge);
                pts[moving_vertex] = Segment3(e).nearest_point(result.point());
                new_position_is_found = true;
            }
        }
        if (!new_position_is_found) for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_point = result.filtered_primitive_id(mode.geom_pts_id(), AttributeBase::TYPE::POINTS);
            if (picked_point != -1) {
                Log::add("need to move vertex to picked point");
                PointSet& geom_pts = mode.geom_pts_id();
                pts[moving_vertex] = geom_pts[picked_point];
                new_position_is_found = true;
            }
        }
    }

    if (!new_position_is_found) {// move paralell to screen if no better solution was found
        Log::add("need to move vertex in 3D");
        PointSet& pts = God::xcf["mesh"].pointset;
        vec3 P = pts[moving_vertex];
        vec3 view_dir = God::camera.pose.forward();

        auto [width, height] = God::context.screen_size();
        vec4 clip_space{ (2.f * p.x) / width - 1.0f, 1.0f - (2.f * p.y) / height, 0, 1.0f };
        vec4 view_space = God::camera.projection_matrix().invert() * clip_space;
        mat4x4 inv_view = God::camera.view_matrix().invert();
        vec4 world_space = inv_view * (view_space / view_space.data[3]);

        pts[moving_vertex] = world_space.xyz() + (view_dir * (P - world_space.xyz())) * view_dir;
    }

    reset();
    mode.update_hex();
    mode.hex_id().broadcast(Event::UPDATED);
}
void InteractorMoveVertex::generate_gui(){
    if (moving_vertex == -1) return;
    std::string msg = "InteractorMoveVertex #" + std::to_string(moving_vertex);
    ImGui::Text(msg.c_str());
}
void InteractorMoveVertex::reset(){
    moving_vertex = -1;
}


InteractorKillHex::InteractorKillHex(BasicEditMode& mode) :mode(mode){}
void InteractorKillHex::on_click(int button, vec2 p) {
    if (!key_state_is_OK()) return;
    if (button != 1) return;
    Picker picker;
    for (auto& d : picker.data) {
        auto result = d.at(p);
        int picked_cell = result.filtered_primitive_id(mode.hex_id(), AttributeBase::TYPE::CELLS);
        if (picked_cell != -1){
            Hexahedra& hex = *(Hexahedra*)mode.hex_id();
            VolumeAttributes& attr = *(VolumeAttributes*)mode.hex_id();
            CellAttribute<int> block_id("block_id", attr, hex);
            int block2kill = block_id[picked_cell];
            std::vector<bool> to_kill(hex.ncells());
            for (auto c : hex.iter_cells()) to_kill[c] = block_id[c] == block2kill;
            hex.delete_cells(to_kill);
            hex.connect();
            mode.update_hex();
            mode.hex_id().broadcast(Event::UPDATED);
        }
    }
}

void InteractorFeedBack::activate(){
    std::string mm_name = "feedback";
    God::xcf.add("feedback");
    int group = God::layers.add_group(mm_name);
    //MultiMesh& mm = God::xcf["feedback"];
    MultiMesh& mm = root.xcf["feedback"];
    mm.polylines.try_emplace("");
    mm.polylines[""].mesh.points = mm.pointset;

    {// point set
        PointSetLayer& layer = God::layers.add<PointSetLayer>(mm_name + "Pts", group);
        layer.init(root.xcf[mm_name].pointset);
        layer.set_default_values();
    }
    for (auto& elt : God::xcf[mm_name].polylines) {
        PolyLineLayer& layer = God::layers.add<PolyLineLayer>(mm_name + "Edges", group);
        layer.init(root.xcf[mm_name].polylines[elt.first]);
        layer.set_default_values();
    }
}
void InteractorFeedBack::desactivate(){
    God::xcf.kill_multimesh("feedback");
}

void InteractorCreateHex::FeedBack::add_point(vec3 v){
    MultiMeshPath mm_id = root.xcf["feedback"];
    if (!mm_id.valid()) activate();
    MultiMesh& mm = God::xcf["feedback"];
    int nv = mm.pointset.create_points(1);
    mm.pointset[nv] = v;
    if (nv > 0){
        PolyLine& pl = mm.polylines[""].mesh;
        pl.disconnect();
        int e = pl.create_edges(1);
        FOR(lv, 2) pl.vert(e, lv) = lv + nv - 1;
        pl.connect();
    }

    root.xcf["feedback"].pointset.broadcast(Event::UPDATED);
    root.xcf["feedback"].polylines[""].broadcast(Event::UPDATED);
}
void InteractorCreateHex::FeedBack::pop_point(){
    MultiMesh& mm = God::xcf["feedback"];
    {
        PolyLine& pl = mm.polylines[""].mesh;
        if (pl.nedges() != 0){
            std::vector<bool> to_kill(pl.nedges(), false);
            to_kill[pl.nedges() - 1] = true;
            pl.delete_edges(to_kill);
            root.xcf["feedback"].polylines[""].broadcast(Event::UPDATED);
        }
    }
    {
        PointSet& pts = mm.pointset;
        if (pts.size() != 0){
            std::vector<bool> to_kill(pts.size(), false);
            to_kill[pts.size() - 1] = true;
            pts.delete_points(to_kill);
            root.xcf["feedback"].pointset.broadcast(Event::UPDATED);
        }
    }
}


InteractorCreateHex::InteractorCreateHex(BasicEditMode& mode) :mode(mode){}
void InteractorCreateHex::reset(){
    if (pts.empty()) return;
    pts.clear();
    feedback.desactivate();
}
void InteractorCreateHex::handle(Event event){
    if (!key_state_is_OK()) return;
    if (event.who == root.xcf["mesh"].hexahedra[""] && event.what_happened == Event::UPDATED) reset();
    Interactor::handle(event);
}
void InteractorCreateHex::on_click(int button, vec2 p) {
    if (!key_state_is_OK()) return;

    if (button != 0) {
        if (!pts.empty()){
            pts.pop_back();
            feedback.pop_point();
        }
        return;
    }
    Picker picker;
    bool picked_a_mesh_vertex = false;
    for (auto& d : picker.data) {
        auto result = d.at(p);
        int picked_vert = result.filtered_primitive_id(mode.pts_id(), AttributeBase::TYPE::POINTS);
        if (picked_vert != -1){
            Hexahedra& hex = mode.hex_id();
            feedback.add_point(hex.points[picked_vert]);
            pts.push_back(hex.points[picked_vert]);
            picked_a_mesh_vertex = true;
        }
    }
    if (!picked_a_mesh_vertex) for (auto& d : picker.data) {
        auto result = d.at(p);
        int picked_triangle = result.filtered_primitive_id(mode.geom_tri_id(), AttributeBase::TYPE::FACETS);
        if (picked_triangle != -1){
            Triangles &tri = mode.geom_tri_id();
            if (pts.size() != 4){
                vec3 P = Triangle3(Surface::Facet(tri,picked_triangle)).nearest_point(result.point());
                pts.push_back(P);
                feedback.add_point(P);
            }
        } else{
            int picked_edge = result.filtered_primitive_id(mode.geom_edge_id(), AttributeBase::TYPE::EDGES);
            if (picked_edge != -1){
                PolyLine& pl = mode.geom_edge_id();
                if (pts.size() != 4){
                    vec3 P = Segment3(PolyLine::Edge(pl, picked_edge)).nearest_point(result.point());
                    pts.push_back(P);
                    feedback.add_point(P);
                }
            }else{
                int picked_point = result.filtered_primitive_id(mode.geom_pts_id(), AttributeBase::TYPE::POINTS);
                if (picked_point != -1){
                    PointSet& ptset = mode.geom_pts_id();
                    if (pts.size() != 4){
                        vec3 P = ptset[picked_point];
                        pts.push_back(P);
                        feedback.add_point(P);
                    }
                }
            }
        }
    }
    if (pts.size() == 4){
        AlgoXCFLevel(mode).create_hex_from_quad(pts, AlgoXCFLevel::RAYTRACE, AlgoXCFLevel::AUTO_MERGE);
        reset();
    }

}
void InteractorCreateHex::generate_gui(){
    if (pts.empty()) return;
    std::string msg = "InteractorCreateHex\nSelected vertices:\n";
    for (auto v : pts) msg += "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")\n";
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), msg.c_str());
}


BasicEditMode::BasicEditMode(HexModelerMode& mode)
    : HexModelerSubMode(mode),
    interactor_kill_kex(*this),
    interactor_create_hex(*this),
    interactor_layer_selection(*this),
    interactor_move_vertex(*this){

    interactor_kill_kex.ctrl_must_be_up = true;
    interactor_create_hex.ctrl_must_be_up = true;
    interactor_camera.ctrl_must_be_down = true;
    interactor_layer_selection.ctrl_must_be_down = true;
    interactor_move_vertex.ctrl_must_be_up = true;

    interactors.push_back(&interactor_kill_kex);
    interactors.push_back(&interactor_create_hex);
    interactors.push_back(&interactor_camera);
    interactors.push_back(&interactor_layer_selection);
    interactors.push_back(&interactor_move_vertex);
}
void BasicEditMode::handle(Event event) {
    Mode::handle(event);
}
void BasicEditMode::define_gui() {
    ImGui::Begin("HexModelerMode", nullptr);
    mode.header_gui_for_submodes();
    generate_interactor_gui();
    ImGui::End();
}
bool BasicEditMode::merge_vertices(int v0, int v1){
    Hexahedra& hex = hex_id();
    FOR(c, hex.ncells())FOR(lv, 8) if (hex.vert(c, lv) == v0)hex.vert(c, lv) = v1;
    return update_hex();
}

