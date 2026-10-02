#pragma once
#include <vector>
#include <string>
#include <queue>
#include <iostream>

#include <ultimaille/all.h>
using namespace UM;

extern const std::string chunk_xcf;
extern const std::string chunk_pointset;
extern const std::string chunk_polylines;
extern const std::string chunk_triangles;
extern const std::string chunk_quads;
extern const std::string chunk_polygons;
extern const std::string chunk_tetrahedra;
extern const std::string chunk_hexahedra;
extern const std::string chunk_wedges;
extern const std::string chunk_pyramids;
extern const std::string chunk_layer;
extern const std::string chunk_camera;
extern const std::string chunk_mouse;
extern const std::string chunk_key;
extern const std::string chunk_panel;


enum EventType {
    CREATED, KILLED, UPDATED
};

struct RenderLayer;
struct Panel;
struct ObjectId {
    std::vector<std::string> chunks;
    void* ptr();

    operator Panel& ();
    operator RenderLayer& ();
    operator PointSet& ();
    operator PointSetAttributes& ();
    operator PolyLine& ();
    operator PolyLineAttributes& ();
    operator Triangles& ();
    operator Quads& ();
    operator SurfaceAttributes& ();

    void emit(EventType e);
    inline void show() {
        for (auto c : chunks) std::cerr << " ==> " << c;
        std::cerr << std::endl;
    }
};

inline bool operator==(const ObjectId& a, const  ObjectId& b) {
    return a.chunks == b.chunks;
}
inline bool operator<(const ObjectId& a, const  ObjectId& b) {
    return a.chunks < b.chunks;
}

struct Event {
    ObjectId who;
    EventType what_happened;
};

struct EventManager {
    void dispatch();
    std::queue<Event> queue;
};

