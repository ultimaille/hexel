#include <modes/submode_basic_edit.h>
#include <modes/mode_hex_modeler.h>






MouseReactKillHex::MouseReactKillHex(BasicEditMode& mode) :mode(mode){}
void MouseReactKillHex::on_click(int button, vec2 p) {
    if (!key_state_is_OK()) return;
    if (button != 1) return;
    Picker picker;
    for (auto& d : picker.data) {
        auto result = d.at(p);
        int picked_cell = result.filtered_primitive_id(mode.hex_id(), 1);
        if (picked_cell != -1){
            Hexahedra& hex = *(Hexahedra *)mode.hex_id();
            VolumeAttributes& attr = *(VolumeAttributes *)mode.hex_id();
            CellAttribute<int> block_id("block_id", attr, hex);
            int block2kill = block_id[picked_cell];
            std::vector<bool> to_kill(hex.ncells());
            for (auto c : hex.iter_cells()) to_kill[c] = block_id[c] == block2kill;
            hex.delete_cells(to_kill);
            hex.connect();
            mode.update_hex();
            mode.hex_id().broadcast(UPDATED);
        }
    }
}










void InteractorFeedBack::activate(vec3 v){
    std::string mm_name = "feedback";
    God::xcf.add("feedback");
    int group = God::layers.add_group(mm_name);
    MultiMesh& mm = God::xcf["feedback"];
    mm.polylines.try_emplace("");
    mm.polylines[""].mesh.points = mm.pointset;

    {// point set
        PointSetLayer& layer = God::layers.add<PointSetLayer>(mm_name + "Pts", group);
        layer.init(ObjectId(POINTSET, mm_name));
        layer.set_default_values();
    }
    for (auto& elt : God::xcf[mm_name].polylines) {
        PolyLineLayer& layer = God::layers.add<PolyLineLayer>(mm_name + "Edges", group);
        layer.init(ObjectId(POLYLINES, { mm_name, elt.first }));
        layer.set_default_values();
    }
}
void InteractorFeedBack::add_point(vec3 v){
    ObjectId mm_id(events::MULTIMESH, "feedback");
    if (!mm_id.ref().has_value()) activate(v);
    MultiMesh& mm = God::xcf["feedback"];
    int nv = mm.pointset.create_points(1);
    mm.pointset[nv] = v;
    if (nv > 0){
        PolyLine& pl= mm.polylines[""].mesh;
        pl.disconnect();
        int e = pl.create_edges(1);
        FOR(lv,2) pl.vert(e, lv) = lv+nv - 1;
        pl.connect();
    }
    
    ObjectId(events::POINTSET, "feedback").broadcast(UPDATED);
    ObjectId(events::POLYLINES, std::vector<std::string>{ "feedback",""}).broadcast(UPDATED);
}

void InteractorFeedBack::pop_point(){
    ObjectId mm_id(events::MULTIMESH, "feedback");
    if (!mm_id.ref().has_value()) return;
    MultiMesh& mm = God::xcf["feedback"];
    {
        PolyLine& pl = mm.polylines[""].mesh;
        if (pl.nedges() != 0){
            std::vector<bool> to_kill(pl.nedges(), false);
            to_kill[pl.nedges() - 1] = true;
            pl.delete_edges(to_kill);
            ObjectId(events::POLYLINES, std::vector<std::string>{ "feedback", ""}).broadcast(UPDATED);
        }
    }
    {
        PointSet& pts = mm.pointset;
        if (pts.size() != 0){
            std::vector<bool> to_kill(pts.size(), false);
            to_kill[pts.size() - 1] = true;
            pts.delete_points(to_kill);
            ObjectId(events::POINTSET, "feedback").broadcast(UPDATED);
        }
    }
}

void InteractorFeedBack::desactivate(){
    God::xcf.kill_multimesh("feedback");
}









MouseReactCreateHex::MouseReactCreateHex(BasicEditMode& mode) :mode(mode){}
void MouseReactCreateHex::reset(){
    if (pts.empty()) return;
    pts.clear();
    feedback.desactivate();
}
void MouseReactCreateHex::handle(Event event){
    if (!key_state_is_OK()) return;

    plop(root().xcf["mesh"].hexahedra[""].path);
    if (Path(event.who) == root().xcf["mesh"].hexahedra[""] && event.what_happened == events::UPDATED) reset();
    MouseReact::handle(event);
}
void MouseReactCreateHex::on_click(int button, vec2 p) {
    if (!key_state_is_OK()) return;
    
    if (button != 0) {
        if (!pts.empty()){
            pts.pop_back();
            feedback.pop_point();
            return;
        }
    }
    Picker picker;
    bool picked_a_mesh_vertex = false;
    for (auto& d : picker.data) {
        auto result = d.at(p);
        int picked_vert = result.filtered_primitive_id(mode.pts_id(), 0);
        if (picked_vert != -1){
            Hexahedra& hex = mode.hex_id();
            feedback.add_point(hex.points[picked_vert]);
            pts.push_back(hex.points[picked_vert]);
            picked_a_mesh_vertex = true;
        }
    }
    if (!picked_a_mesh_vertex) for (auto& d : picker.data) {
            auto result = d.at(p);
            int picked_triangle = result.filtered_primitive_id(mode.geom_tri_id(), 1);
        if (picked_triangle != -1){
            Hexahedra& hex = mode.hex_id();
            if (pts.size() != 4){
                pts.push_back(result.point());
                feedback.add_point(result.point());
            }
        }
    }

    MultiMesh &mm =ObjectId(events::MULTIMESH, "mesh");
    CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);


    
        if (pts.size() == 4){
            Hexahedra& hex = mode.hex_id();
            int offset = hex.points.create_points(8);
            FOR(lv, 4) hex.points[offset + lv] = pts[lv];

            int c = hex.create_cells(1);
            block_id[c] = c;
            hex.vert(c, 0) = offset+0;
            hex.vert(c, 1) = offset + 1;
            hex.vert(c, 2) = offset + 3;
            hex.vert(c, 3) = offset + 2;
            FOR(lv, 4) hex.vert(c, 4 + lv) = offset +4+ lv;
            FOR(lv, 4) hex.points[offset +4+ lv] = hex.points[hex.vert(c, lv)];
            mode.update_hex();
            mode.hex_id().broadcast(UPDATED);
        }

}

void MouseReactCreateHex::generate_gui(){
    if (!key_state_is_OK()) return;
    std::string msg = "MouseReactCreateHex\nSelected vertices:\n";
    for (auto v : pts) msg += "(" + std::to_string(v.x) + "," + std::to_string(v.y) + "," + std::to_string(v.z) + ")\n";
    ImGui::Text(msg.c_str());
}



BasicEditMode::BasicEditMode(HexModelerMode& mode) 
    : HexModelerSubMode(mode), interactor_kill_kex(*this), interactor_create_hex(*this), interactor_layer_selection(*this){
    interactor_kill_kex.ctrl_must_be_up = true;
    interactor_create_hex.ctrl_must_be_up = true;
    interactor_camera.ctrl_must_be_down = true;
    interactor_layer_selection.ctrl_must_be_down = true;
    interactors.push_back(&interactor_kill_kex);
    interactors.push_back(&interactor_create_hex);
    interactors.push_back(&interactor_camera);
    interactors.push_back(&interactor_layer_selection);
}


void BasicEditMode::handle(Event event) {
    Mode::handle(event);
}

void BasicEditMode::define_gui() {
    ImGui::Begin("HexModelerMode", nullptr);
    mode.header_gui_for_submodes();
    mode.generate_interactor_gui();
    ImGui::End();
}

