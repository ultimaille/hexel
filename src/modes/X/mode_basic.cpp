#include <modes/X/cmd.h>
#include <modes/X/mode_basic.h>
#include <modes/X/root_mode.h>
#include <modes/X/toolbox.h>

namespace ModeX{

        BasicInteractor::BasicInteractor(SubModeBasic& mode) : mode(mode){}
        bool BasicInteractor::is_locked(){ return mode.has_lock != NULL; }
        bool BasicInteractor::have_lock(){ return mode.has_lock == this; }
        void BasicInteractor::get_lock(){ um_assert(!is_locked()); mode.has_lock = this; }
        void BasicInteractor::release_lock(){ um_assert(have_lock()); mode.has_lock = NULL; }



    InteractorMoveVertex::InteractorMoveVertex(SubModeBasic& mode) :BasicInteractor(mode){ moving_vertex = -1; }
    void InteractorMoveVertex::on_press(int button, vec2 p) {
        if (!key_state_is_OK()) return;
        if (button != 0) return;
        if (is_locked() && !have_lock()) return;

        picker.reset();
        for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_vert = result.filtered_primitive_id(mode.pts_id(), AttributeBase::TYPE::POINTS);
            if (picked_vert != -1) {
                moving_vertex = picked_vert;
                get_lock();
                return;
            }
        }
    }

    void InteractorMoveVertex::on_release(int button, vec2 p) {
        if (!key_state_is_OK()) return;
        if (button != 0) return;
        if (!have_lock()) {
            moving_vertex = -1; 
            return;
        }
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
                Commands(mode).merge_vertices(moving_vertex, target_vertex);
                reset();
                return;
            }
        }
        bool new_position_is_found = false;
        vec3 new_pos;
        if (mode.drop_vertices_on_geometry)
        {// try to move the vertex on the geometry
            for (auto& d : picker.data) {
                auto result = d.at(p);
                int picked_triangle = result.filtered_primitive_id(mode.geom_tri_id(), AttributeBase::TYPE::FACETS);
                if (picked_triangle != -1) {
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
                    PointSet& geom_pts = mode.geom_pts_id();
                    new_pos = geom_pts[picked_point];
                    new_position_is_found = true;
                }
            }
        }

        if (!new_position_is_found) {// move paralell to screen if no better solution was found
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
        Commands(mode).move_vertex(moving_vertex, new_pos);
        reset();
    }
    void InteractorMoveVertex::generate_gui(){
        if (!have_lock()) return;
        if (moving_vertex == -1) return;
        std::string msg = "InteractorMoveVertex #" + std::to_string(moving_vertex);
        ImGui::Text(msg.c_str());
    }
    void InteractorMoveVertex::reset(){
        moving_vertex = -1;
        release_lock();
    }


    InteractorKillHex::InteractorKillHex(SubModeBasic& mode) :BasicInteractor(mode){}
    void InteractorKillHex::on_click(int button, vec2 p) {
        if (!key_state_is_OK()) return;
        if (button != 1) return;
        if (is_locked()) return;
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
                Commands(mode).kill_hexes(to_kill);
            }
        }
    }





    InteractorCreateHex::InteractorCreateHex(SubModeBasic& mode) :BasicInteractor(mode){}
    void InteractorCreateHex::reset(){
        if (have_lock()) release_lock();
        if (pts.empty()) return;
        pts.clear();
        feedback.desactivate();
    }
    void InteractorCreateHex::handle(Event event){
        if (!key_state_is_OK()) return;
        //if (event.who == root.xcf["mesh"].hexahedra[""] && event.what_happened == Event::UPDATED) reset();
        Interactor::handle(event);
    }

    void InteractorCreateHex::update_feedback(){
        
        if (pts.empty()) {
            feedback.desactivate();
            return;
        }
        if (pts.size() == 1) if (!have_lock()) get_lock();
        um_assert(have_lock());
        if (!feedback.activated()) feedback.activate();
        feedback.begin_mesh_update();
        feedback.add_polyline(pts);
        feedback.end_mesh_update();
    }

    void InteractorCreateHex::on_click(int button, vec2 p) {
        if (!key_state_is_OK()) return;
        if (is_locked() && !have_lock()) return;
        if (button != 0) {
            if (!pts.empty()){
                pts.pop_back();
                update_feedback();
                if (pts.empty()) 
                    release_lock();
            }
            return;
        }
        Picker picker;
        // create an hex from a picked facet... may not be always avalaible (e.g. check group merge fct) 
        if (pts.empty() && !is_locked() && !have_lock()) for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_facet = result.filtered_primitive_id(mode.hex_id(), AttributeBase::TYPE::CELLFACETS);
            if (picked_facet != -1){
                Commands(mode).extend_mesh_from_face(picked_facet);
                reset();
                return;
            }
        }



        bool picked_a_mesh_vertex = false;
        for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_vert = result.filtered_primitive_id(mode.pts_id(), AttributeBase::TYPE::POINTS);
            if (picked_vert != -1){
                Hexahedra& hex = mode.hex_id();
                pts.push_back(hex.points[picked_vert]);
                update_feedback(); 
                picked_a_mesh_vertex = true;
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
                    update_feedback();
                }
            }
            else{
                int picked_edge = result.filtered_primitive_id(mode.geom_edge_id(), AttributeBase::TYPE::EDGES);
                if (picked_edge != -1){
                    PolyLine& pl = mode.geom_edge_id();
                    if (pts.size() != 4){
                        vec3 P = Segment3(PolyLine::Edge(pl, picked_edge)).nearest_point(result.point());
                        pts.push_back(P);
                        update_feedback();
                    }
                }
                else{
                    int picked_point = result.filtered_primitive_id(mode.geom_pts_id(), AttributeBase::TYPE::POINTS);
                    if (picked_point != -1){
                        PointSet& ptset = mode.geom_pts_id();
                        if (pts.size() != 4){
                            vec3 P = ptset[picked_point];
                            pts.push_back(P);
                            update_feedback();
                        }
                    }
                }
            }
        }
        if (pts.size() == 4){
            Commands(mode).create_hex_from_quad(pts, Commands::RAYTRACE, Commands::AUTO_MERGE);
            reset();
        }

    }
    void InteractorCreateHex::generate_gui(){
        if (pts.empty()) return;

        std::string msg = "InteractorCreateHex\nSelected vertices:\n";
        for (auto v : pts) msg += "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")\n";
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), msg.c_str());
    }






    SubModeBasic::SubModeBasic(RootMode& mode)
        : AbstractMode(mode),
        interactor_kill_kex(*this),
        interactor_create_hex(*this),
        interactor_move_vertex(*this){

        interactor_kill_kex.ctrl_must_be_up = true;
        interactor_create_hex.ctrl_must_be_up = true;
        interactor_camera.ctrl_must_be_down = true;
        interactor_layer_selection.ctrl_must_be_down = true;
        interactor_move_vertex.ctrl_must_be_up = true;

        interactors.push_back(&interactor_kill_kex);
        interactors.push_back(&interactor_create_hex);
        interactors.push_back(&interactor_move_vertex);


    }
    void SubModeBasic::handle(Event event) {
        Mode::handle(event);
    }
    void SubModeBasic::define_gui() {

        ImGui::Checkbox("Try to land vertices on geometry", &drop_vertices_on_geometry);
        
        ImGui::Separator();
        if (has_lock != NULL){
            std::string msg;
            if (has_lock == &interactor_move_vertex)
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ACTIVE interactor: MoveVertex");
            if (has_lock == &interactor_create_hex)
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ACTIVE interactor: CreateHex");
        }
        generate_interactor_gui();
    }

};
