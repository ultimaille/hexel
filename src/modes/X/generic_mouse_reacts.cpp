#include "generic_mouse_reacts.h"
#include <core/all.h>
#include <algo/raster_iterator.h>
#include <modes/X/abstract_mode.h>
#include <modes/X/root_mode.h>
namespace ModeX{
    void InteractorLayerSelection::on_click(int button, vec2 p) {
        if (!key_state_is_OK()) return;
        if (button != 0) return;
        Picker& picker = mode.mode.picker;
        FOR(buf,picker.nbuffers()){
            Picker::Result pr = picker.at(p,buf);
            int prim = pr.filtered_primitive_id(mode.hex_id(), AttributeBase::TYPE::CELLFACETS);
            if (prim != -1){
                Hexahedra& hex = *(Hexahedra*)mode.hex_id();
                CellFacetAttribute<int> layer_id(hex);
                compute_hex_layer_id(hex, layer_id);

                VolumeAttributes& hex_attr = *(VolumeAttributes*)mode.hex_id();
                CellAttribute<bool> visible_cell("visible", hex_attr, hex);
                visible_cell.fill(false);
                for (auto f : hex.iter_facets()) if (layer_id[f] == layer_id[prim])
                    visible_cell[f.cell()] = true;

                PointSetAttributes& pts_attr = *(PointSetAttributes*)mode.pts_id();
                PointSet& pts = *(PointSet*)mode.pts_id();
                PointAttribute<bool> visible_pts("visible", pts_attr, pts);
                visible_pts.fill(false);
                for (auto c : hex.iter_cells()) if (visible_cell[c])
                    FOR(lv, 8)visible_pts[c.vertex(lv)] = true;


                PolyLineAttributes& pl_attr = *(PolyLineAttributes*)mode.pl_id();
                PolyLine& pl = *(PolyLine*)mode.pl_id();
                EdgeAttribute<bool> visible_edge("visible", pl_attr, pl);
                visible_edge.fill(false);
                for (auto e : pl.iter_edges()) if (visible_pts[e.from()] && visible_pts[e.to()])
                    visible_edge[e] = true;

                mode.hex_id().broadcast(Event::UPDATED);
                mode.pl_id().broadcast(Event::UPDATED);
                mode.pts_id().broadcast(Event::UPDATED);
                return;
            }
        }
        {
            CellAttribute<bool> visible_cell("visible", *(VolumeAttributes*)mode.hex_id(), *(Hexahedra*)mode.hex_id());
            visible_cell.fill(true);
            PointAttribute<bool> visible_pts("visible", *(PointSetAttributes*)mode.pts_id(), *(PointSet*)mode.pts_id());
            visible_pts.fill(true);
            EdgeAttribute<bool> visible_edge("visible", *(PolyLineAttributes*)mode.pl_id(), *(PolyLine*)mode.pl_id());
            visible_edge.fill(true);
            mode.hex_id().broadcast(Event::UPDATED);
            mode.pl_id().broadcast(Event::UPDATED);
            mode.pts_id().broadcast(Event::UPDATED);
            return;
        }
    }

};