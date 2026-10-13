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