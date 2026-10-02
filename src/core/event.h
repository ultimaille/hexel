#pragma once
#include <vector>
#include <string>
#include <queue>
#include <variant>
#include <optional>
#include <iostream>

#include <ultimaille/all.h>
#include "xcf.h"

using namespace UM;

struct KeyboardState;
struct MouseState;
struct Camera;
struct RenderLayer;

namespace events {
    enum ObjectType { NA, KEYBOARD, MOUSE, CAMERA, LAYER, XCF, POINTSET, POLYLINES, TRIANGLES, QUADS, POLYGONS, TETRAHEDRA, HEXAHEDRA, WEDGES, PYRAMIDS };
    enum EventType {
        CREATED, KILLED, UPDATED
    };
}

struct ObjectId {
    ObjectId() = default;
    ObjectId(events::ObjectType path, std::vector<std::string> names={}) : path(path), names(names) {}
    ObjectId(events::ObjectType path, std::string name) : path(path), names(1, name) {}

    using ObjectRef = std::variant<
        std::reference_wrapper<KeyboardState>,
        std::reference_wrapper<MouseState>,
        std::reference_wrapper<Camera>,
        std::reference_wrapper<RenderLayer>,
        std::reference_wrapper<PointSet>,
        std::reference_wrapper<PointSetAttributes>,
        std::reference_wrapper<MultiMesh::MeshAttr<PolyLine,   PolyLineAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Triangles,  SurfaceAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Quads,      SurfaceAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Polygons,   SurfaceAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Hexahedra,  VolumeAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Wedges,     VolumeAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Pyramids,   VolumeAttributes>>
            >;

    std::optional<ObjectRef> ref();  // returns std::nullopt if path invalid

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

    void broadcast(events::EventType e);

    events::ObjectType path = events::NA;
    std::vector<std::string> names = {}; // multimesh name, mesh name, etc.

private:
    template<typename T> T& as() {
        std::optional<ObjectRef> v = ref();
        um_assert(v.has_value());
        auto* p = std::get_if<std::reference_wrapper<T>>(&*v);
        um_assert(p != nullptr);
        return p->get();
    }
};

inline bool operator==(const ObjectId& a, const ObjectId& b) {
    return a.path == b.path && a.names == b.names;
}

inline bool operator==(const ObjectId& o, const events::ObjectType& p) {
    return o.path == p && !o.names.size();
}

inline bool operator==(const events::ObjectType& p, const ObjectId& o) {
    return o.path == p && !o.names.size();
}

struct Event {
    ObjectId who;
    events::EventType what_happened;
};

struct EventManager {
    void dispatch();
    std::queue<Event> queue;
};

