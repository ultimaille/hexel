#pragma once
#include <vector>
#include <string>
#include <queue>
#include <variant>
#include <optional>
#include <iostream>

#include <ultimaille/all.h>

using namespace UM;

struct KeyboardState;
struct MouseState;
struct Camera;
struct RenderLayer;

struct ObjectId {
    enum Type { KEYBOARD, MOUSE, CAMERA, LAYER, XCF, POINTSET, POLYLINES, TRIANGLES, QUADS, POLYGONS, TETRAHEDRA, HEXAHEDRA, WEDGES, PYRAMIDS };

    using ObjectPtr = std::variant<
        std::reference_wrapper<KeyboardState>,
        std::reference_wrapper<MouseState>,
        std::reference_wrapper<Camera>,
        std::reference_wrapper<RenderLayer>,
        std::reference_wrapper<PointSet>,
        std::reference_wrapper<PointSetAttributes>,
        std::reference_wrapper<PolyLine>,
        std::reference_wrapper<Triangles>,
        std::reference_wrapper<Quads>,
        std::reference_wrapper<Polygons>,
        std::reference_wrapper<SurfaceAttributes>,
        std::reference_wrapper<Tetrahedra>,
        std::reference_wrapper<Hexahedra>,
        std::reference_wrapper<Wedges>,
        std::reference_wrapper<Pyramids>,
        std::reference_wrapper<VolumeAttributes>
            >;

    std::optional<ObjectPtr> ptr();  // returns std::nullopt if path invalid

    operator KeyboardState&();
    operator MouseState&();
    operator Camera&();
    operator RenderLayer&();

    operator PointSet&();
    operator PointSetAttributes&();
    operator PolyLine&();
    operator PolyLineAttributes&();
    operator Triangles&();
    operator Quads&();
    operator Polygons&();
    operator SurfaceAttributes&();
    operator Tetrahedra&();
    operator Hexahedra&();
    operator Wedges&();
    operator Pyramids&();
    operator VolumeAttributes&();

    void emit(EventType e);
    inline void show() {
        for (auto c : chunks) std::cerr << " ==> " << c;
        std::cerr << std::endl;
    }

    std::vector<Type> path;
    std::vector<std::string> names; // multimesh name, mesh name, etc.
};

inline bool operator==(const ObjectId& a, const  ObjectId& b) {
    return a.path == b.path && a.names == b.names;
}

struct Event {
    enum EventType {
        CREATED, KILLED, UPDATED
    };

    ObjectId who;
    EventType what_happened;
};

struct EventManager {
    void dispatch();
    std::queue<Event> queue;
};

