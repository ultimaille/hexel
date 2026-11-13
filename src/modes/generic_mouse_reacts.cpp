#include "generic_mouse_reacts.h"
#include <core/all.h>
#include <algo/raster_iterator.h>
#include <modes/mode_hex_modeler.h>

void InteractorLayerSelection::on_click(int button, vec2 p) {
        if (!key_state_is_OK()) return;
        if (button != 0) return;
        Picker picker;
        Picker::Result pr = picker.at(p);
        int prim = pr.filtered_primitive_id(hexmode.hex_id(), 2);
        if (prim != -1){
            Hexahedra& hex = *(Hexahedra *)hexmode.hex_id();
            CellFacetAttribute<int> layer_id(hex);
            compute_hex_layer_id(hex, layer_id);

            VolumeAttributes& hex_attr = *(VolumeAttributes *)hexmode.hex_id();
            CellAttribute<bool> visible_cell("visible", hex_attr, hex);
            visible_cell.fill(false);
            for (auto f : hex.iter_facets()) if (layer_id[f] == layer_id[prim])
                visible_cell[f.cell()] = true;
            //hex_id.broadcast(EventType::UPDATED);


            PointSetAttributes& pts_attr = *(PointSetAttributes *)hexmode.pts_id();
            PointSet& pts = *(PointSet *)hexmode.pts_id();
            PointAttribute<bool> visible_pts("visible", pts_attr, pts);
            visible_pts.fill(false);
            for (auto c : hex.iter_cells()) if (visible_cell[c])
                FOR(lv, 8)visible_pts[c.vertex(lv)] = true;


            PolyLineAttributes& pl_attr = *(PolyLineAttributes *)hexmode.pl_id();
            PolyLine& pl = *(PolyLine *)hexmode.pl_id();
            EdgeAttribute<bool> visible_edge("visible", pl_attr, pl);
            visible_edge.fill(false);
            for (auto e : pl.iter_edges()) if (visible_pts[e.from()] && visible_pts[e.to()])
                visible_edge[e] = true;

            hexmode.hex_id().broadcast(EventType::UPDATED);
            hexmode.pl_id().broadcast(EventType::UPDATED);
            hexmode.pts_id().broadcast(EventType::UPDATED);
        }
    }

