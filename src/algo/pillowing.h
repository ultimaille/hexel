#pragma once
#include <ultimaille/all.h>

using namespace UM;
using namespace UM::Linear;

struct Pillow{
    Pillow(Hexahedra& hex, CellFacetAttribute<int>& pillow);

    int  map_corners_to_output_vertices(CellCornerAttribute<int>& c2ov);

    std::vector<std::pair<int, int> > get_output_vertices_around_edge(CellCornerAttribute<int>& c2ov, EdgeGraph& eg, EdgeGraph::Edge e);

    void pillow_without_boundary_faces(double thickness);

    void pad();

    void unpad();

    void pillow_with_boundary_faces(double thickness);

    void apply(double thickness = .5);

    Hexahedra& hex;
    CellFacetAttribute<int>& pillow;

};

