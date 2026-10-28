#pragma once 
#include "misc/purgatory.h"

inline int compute_hex_layer_id(Hexahedra& hex, CellFacetAttribute<int>& layer_id){
    if (!hex.connected()) hex.connect();
    DisjointSet ds(hex.nfacets());
    for (auto h : hex.iter_halfedges()){
        auto opp = h.opposite_c();
        if (!opp.active()) continue;
        ds.merge(h.opposite_f().facet(), opp.opposite_f().facet());
    }
    return ds.get_sets_id(layer_id.ptr->data);
}


void generate_edge_graph(Hexahedra& hex, PolyLine& pl, EdgeAttribute<int>& hex_halfedge, bool remove_isolated_vertices = true) {
    std::vector<bool> to_kill(pl.nedges(), true);
    pl.delete_edges(to_kill);
    if (remove_isolated_vertices)
        hex.delete_isolated_vertices();
    if (!hex.connected()) hex.connect();
    DisjointSet ds(hex.nhalfedges());
    for (auto h : hex.iter_halfedges())
        if (h.facet().opposite().active())
            ds.merge(h, h.opposite_c().opposite_f());

    std::vector<int> edges_;
    int nedges = ds.get_sets_id(edges_);
    pl.create_edges(nedges);
    hex_halfedge.fill(-1);
    for (auto h : hex.iter_halfedges()) {
        int e = edges_[h];
        pl.vert(e, 0) = h.from();
        pl.vert(e, 1) = h.to();
        hex_halfedge[e] = h;
    }
    pl.connect();
}