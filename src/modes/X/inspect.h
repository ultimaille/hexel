#pragma once
//#include "misc/purgatory.h"
//#include <layers/ssao.h>
//#include <layers/basic_layers.h>
//#include "algo.h"
//#include <algo/pillowing.h>
//#include <algo/raster_iterator.h>
//#include <modes/X/generic_mouse_reacts.h>
#include <modes/X/scene.h>
#include <core/mode.h>

namespace ModeX{
    struct RootMode;
    struct Inspect : public XCFview{
        Inspect(RootMode& mode) : XCFview(mode){}

        struct Embedding{ int dim = -1; int id = -1; };
        inline bool on_point(vec3 P, vec3 point){ return (P - point).norm2() < 1e-20; }
        inline bool on_edge(vec3 P, Segment3 s){ return (P - s.nearest_point(P)).norm2() < 1e-15; }
        inline bool on_triangle(vec3 P, Triangle3 tr){ return (P - tr.nearest_point(P)).norm2() < 1e-15; }

        inline Embedding get_embedding(vec3 P) {
            Triangles& tri = geom_tri_id();
            PolyLine& pl = geom_edge_id();
            for (auto v : tri.iter_vertices()) if (on_point(v.pos(), P)) return { 0,v };
            for (auto e : pl.iter_edges())if (on_edge(P, e)) return { 1,e };
            for (auto f : tri.iter_facets())if (on_triangle(P, f)) return { 2,f };
            return { -1,-1 };
        }
    };

}

