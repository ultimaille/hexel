#include <modes/mode_hex_modeler.h>



struct MouseReactSplitAddFeature : public MouseReact {
    MouseReactSplitAddFeature() {}
    void init(ObjectId p_pl, ObjectId p_pts) { pl_id = p_pl; pts_id = p_pts; }
    ObjectId pl_id;
    ObjectId pts_id;

    inline void on_click(int button, vec2 p) {
        if (button != 0) return;
        Picker picker;
        Picker::Result pr = picker.at(p);
        int prim = pr.filtered_primitive_id(pts_id, 0);
        if (prim != -1){
            plop("facet_vert_iter seclected");
            //Hexahedra& hex = hex_id;
            //CellFacetAttribute<int> layer_id(hex);
            //compute_hex_layer_id(hex, layer_id);

            //VolumeAttributes& hex_attr = hex_id;
            //CellAttribute<bool> visible_cell("visible", hex_attr, hex);
            //visible_cell.fill(false);
            //for (auto f : hex.iter_facets()) if (layer_id[f] == layer_id[prim])
            //    visible_cell[f.cell()] = true;
            //hex_id.broadcast(EventType::UPDATED);


            //PointSetAttributes& pts_attr = pts_id;
            //PointSet& pts = pts_id;
            //PointAttribute<bool> visible_pts("visible", pts_attr, pts);
            //visible_pts.fill(false);
            //for (auto c : hex.iter_cells()) if (visible_cell[c])
            //    FOR(lv, 8)visible_pts[c.vertex(lv)] = true;


            //PolyLineAttributes& pl_attr = pl_id;
            //PolyLine& pl = pl_id;
            //EdgeAttribute<bool> visible_edge("visible", pl_attr, pl);
            //visible_edge.fill(false);
            //for (auto e : pl.iter_edges()) if (visible_pts[e.from()] && visible_pts[e.to()])
            //    visible_edge[e] = true;

            //hex_id.broadcast(EventType::UPDATED);
            //pl_id.broadcast(EventType::UPDATED);
            //pts_id.broadcast(EventType::UPDATED);
        }
    }
};



struct FeatureEditMode : public HexModelerMode::SubMode{

    MouseReactMultiSelector multi_selector_react_left;
    MouseReactMultiSelector multi_selector_react_right;
    MouseReactSplitAddFeature split_add_react;

    FeatureEditMode(HexModelerMode& mode);
    void activate();
    void define_gui();
    void handle(Event event);
};
