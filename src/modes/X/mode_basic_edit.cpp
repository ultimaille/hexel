#include <modes/X/root_mode.h>
#include <modes/X/mode_basic_edit.h>
#include <modes/X/toolbox.h>
#include <modes/X/cmd.h>






namespace ModeX{

    InteractorMoveVertex::InteractorMoveVertex(SubModeBasicEdit& mode) :mode(mode){ moving_vertex = -1; }
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
                ModeXCommands(mode).merge_vertices(moving_vertex, target_vertex);
                reset();
                return;
            }
        }
        bool new_position_is_found = false;
        vec3 new_pos;
        {// try to move the vertex on the geometry
            for (auto& d : picker.data) {
                auto result = d.at(p);
                int picked_triangle = result.filtered_primitive_id(mode.geom_tri_id(), AttributeBase::TYPE::FACETS);
                if (picked_triangle != -1) {
                    Log::add("need to move vertex to picked triangle");
                    Triangles& tri = mode.geom_tri_id();
                    Surface::Facet f(tri, picked_triangle);
                    new_pos = Triangle3(f).nearest_point(result.point());
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
                    new_pos = Segment3(e).nearest_point(result.point());
                    new_position_is_found = true;
                }
            }
            if (!new_position_is_found) for (auto& d : picker.data) {
                auto result = d.at(p);
                int picked_point = result.filtered_primitive_id(mode.geom_pts_id(), AttributeBase::TYPE::POINTS);
                if (picked_point != -1) {
                    Log::add("need to move vertex to picked point");
                    PointSet& geom_pts = mode.geom_pts_id();
                    new_pos = geom_pts[picked_point];
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

            new_pos = world_space.xyz() + (view_dir * (P - world_space.xyz())) * view_dir;
        }
        ModeXCommands(mode).move_vertex(moving_vertex, new_pos);
        //reset();
        //mode.update_hex();
        //mode.hex_id().broadcast(Event::UPDATED);
    }
    void InteractorMoveVertex::generate_gui(){
        if (moving_vertex == -1) return;
        std::string msg = "InteractorMoveVertex #" + std::to_string(moving_vertex);
        ImGui::Text(msg.c_str());
    }
    void InteractorMoveVertex::reset(){
        moving_vertex = -1;
    }


    InteractorKillHex::InteractorKillHex(SubModeBasicEdit& mode) :mode(mode){}
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
                mode.scene().update_hex();
                mode.hex_id().broadcast(Event::UPDATED);
            }
        }
    }

    void InteractorFeedBack::activate(){
        std::string mm_name = "feedback";
        God::xcf.add("feedback");
        int group = God::layers.add_group(mm_name);
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


    InteractorCreateHex::InteractorCreateHex(SubModeBasicEdit& mode) :mode(mode){}
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

        // create an hex from a picked facet... may not be always avalaible (e.g. check group merge fct) 
        if (pts.empty()) for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_facet = result.filtered_primitive_id(mode.hex_id(), AttributeBase::TYPE::CELLFACETS);
            if (picked_facet != -1){
                ModeXCommands(mode).extend_mesh_from_face(picked_facet);
                reset();
                return;
            }
        }


        if (!picked_a_mesh_vertex) for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_triangle = result.filtered_primitive_id(mode.geom_tri_id(), AttributeBase::TYPE::FACETS);
            if (picked_triangle != -1){
                Triangles& tri = mode.geom_tri_id();
                if (pts.size() != 4){
                    vec3 P = Triangle3(Surface::Facet(tri, picked_triangle)).nearest_point(result.point());
                    pts.push_back(P);
                    feedback.add_point(P);
                }
            }
            else{
                int picked_edge = result.filtered_primitive_id(mode.geom_edge_id(), AttributeBase::TYPE::EDGES);
                if (picked_edge != -1){
                    PolyLine& pl = mode.geom_edge_id();
                    if (pts.size() != 4){
                        vec3 P = Segment3(PolyLine::Edge(pl, picked_edge)).nearest_point(result.point());
                        pts.push_back(P);
                        feedback.add_point(P);
                    }
                }
                else{
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
            ModeXCommands(mode).create_hex_from_quad(pts, ModeXCommands::RAYTRACE, ModeXCommands::AUTO_MERGE);
            reset();
        }

    }
    void InteractorCreateHex::generate_gui(){
        if (pts.empty()) return;
        std::string msg = "InteractorCreateHex\nSelected vertices:\n";
        for (auto v : pts) msg += "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")\n";
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), msg.c_str());
    }






    SubModeBasicEdit::SubModeBasicEdit(RootMode& mode)
        : AbstractMode(mode),
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
    void SubModeBasicEdit::handle(Event event) {
        Mode::handle(event);
    }
    void SubModeBasicEdit::define_gui() {
        //ImGui::Begin("HexModelerMode", nullptr);
        //mode.header_gui_for_submodes();
        generate_interactor_gui();
        //ImGui::End();
    }
    bool SubModeBasicEdit::merge_vertices(int v0, int v1){
        Hexahedra& hex = hex_id();
        FOR(c, hex.ncells())FOR(lv, 8) if (hex.vert(c, lv) == v0)hex.vert(c, lv) = v1;
        return scene().update_hex();
    }
};
