#pragma once
#include <core/all.h>

struct FeedBack3D{
    MultiMeshPath::PointSet pts_id();
    MultiMeshPath::PolyLines::PolyLine pl_id();
    MultiMeshPath::Surfaces::Surface tri_id();

    FeedBack3D(std::string name = "feedback") :name(name){}
    std::string name;
    bool activated = false;
    void activate();
    void desactivate();
    void begin_mesh_update();
    void end_mesh_update();
    void add_polyline(std::vector<vec3>& verts);
    void add_points(std::vector<vec3>& verts);
};

