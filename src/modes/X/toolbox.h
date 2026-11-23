#pragma once
#include <core/basic.h>

template<class T> struct ToolBox{ ToolBox(T& m) {}; };






struct ToolBoxHexahedra{
    ToolBoxHexahedra(Hexahedra& hex);
    Hexahedra& hex;
    void merge_vertices(int source, int target);
    bool validate_topo(std::vector<int>& conficts);
    bool validate_geom(std::vector<int>& conficts);
};

template<> struct ToolBox<Hexahedra> :public ToolBoxHexahedra{
    ToolBox(Hexahedra& hex) :ToolBoxHexahedra(hex){};
};









struct ToolBoxTriangles{
    ToolBoxTriangles(Triangles& tri);
    Triangles& tri;
    vec3 next_intersection(vec3 O, vec3 dir);
};

template<> struct ToolBox<Triangles> :public ToolBoxTriangles{
    ToolBox(Triangles& tri) :ToolBoxTriangles(tri){};
};

