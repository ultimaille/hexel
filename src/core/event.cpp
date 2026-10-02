#include "event.h"
#include "core.h"

using namespace events;

void ObjectId::broadcast(EventType e) {
    God::events.queue.emplace(Event(*this, e));
}

std::optional<ObjectId::ObjectRef> ObjectId::ref() {
    switch (path) {
        case KEYBOARD:
            return std::ref(God::keys);

        case MOUSE:
            return std::ref(God::mouse);

        case CAMERA:
            return std::ref(God::camera);

        case PANEL:
            if (names.size() != 1) return std::nullopt;
            if (!God::panels.contains(names[0])) return std::nullopt;
            return std::ref(God::panels[names[0]]);

        case LAYER:
            if (names.size() != 1) return std::nullopt;
            if (!God::layers.contains(names[0])) return std::nullopt;
            return std::ref(God::layers[names[0]]);

        case MULTIMESH:
        case POINTSET: {
            if (names.size() != 1) return std::nullopt;
            std::string mm = names[0];
            if (!God::xcf.contains(mm)) return std::nullopt;
            return std::ref(God::xcf[mm]);
        }

        case POLYLINES:
        case TRIANGLES:
        case QUADS:
        case POLYGONS:
        case TETRAHEDRA:
        case HEXAHEDRA:
        case WEDGES:
        case PYRAMIDS: {
            if (names.size() != 2) return std::nullopt;
            std::string mm = names[0];
            std::string m  = names[1];
            if (!God::xcf.contains(mm)) return std::nullopt;

            auto& multimesh = God::xcf[mm];

            switch (path) {
                case POLYLINES:
                    if (!multimesh.polylines.contains(m)) return std::nullopt;
                    return std::ref(multimesh.polylines[m]);
                case TRIANGLES:
                    if (!multimesh.triangles.contains(m)) return std::nullopt;
                    return std::ref(multimesh.triangles[m]);
                case QUADS:
                    if (!multimesh.quads.contains(m)) return std::nullopt;
                    return std::ref(multimesh.quads[m]);
                case POLYGONS:
                    if (!multimesh.polygons.contains(m)) return std::nullopt;
                    return std::ref(multimesh.polygons[m]);
                case TETRAHEDRA:
                    if (!multimesh.tetrahedra.contains(m)) return std::nullopt;
                    return std::ref(multimesh.tetrahedra[m]);
                case HEXAHEDRA:
                    if (!multimesh.hexahedra.contains(m)) return std::nullopt;
                    return std::ref(multimesh.hexahedra[m]);
                case WEDGES:
                    if (!multimesh.wedges.contains(m)) return std::nullopt;
                    return std::ref(multimesh.wedges[m]);
                case PYRAMIDS:
                    if (!multimesh.pyramids.contains(m)) return std::nullopt;
                    return std::ref(multimesh.pyramids[m]);
                default:
                    um_assert(!"Something is missing here");
            }
        }

        default:
            return std::nullopt;
    }
}


ObjectId::operator Panel& () {
    return as<Panel>();
}

ObjectId::operator RenderLayer& () {
    return as<RenderLayer>();
}

ObjectId::operator MultiMesh&() {
    return as<MultiMesh>();
}

ObjectId::operator PointSet&() {
    return as<MultiMesh>().pointset;
}

ObjectId::operator PointSetAttributes&() {
    return as<MultiMesh>().pointset_attributes;
}

ObjectId::operator PolyLine&() {
    auto &ma = as<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>();
    return ma.mesh;
}

ObjectId::operator PolyLineAttributes&() {
    auto &ma = as<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>();
    return ma.attributes;
}

ObjectId::operator Triangles&() {
    auto &ma = as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>();
    return ma.mesh;
}


ObjectId::operator Quads&() {
    auto &ma = as<MultiMesh::MeshAttr<Quads, SurfaceAttributes>>();
    return ma.mesh;
}


ObjectId::operator Polygons&() {
    auto &ma = as<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>>();
    return ma.mesh;
}

ObjectId::operator SurfaceAttributes&() {
    switch (path) {
        case TRIANGLES: return as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>().attributes;
        case QUADS:     return as<MultiMesh::MeshAttr<Quads,     SurfaceAttributes>>().attributes;
        case POLYGONS:  return as<MultiMesh::MeshAttr<Polygons,  SurfaceAttributes>>().attributes;
        default: um_assert(!"invalid cast");
    }
    static SurfaceAttributes dummy;
    return dummy;
}

void EventManager::dispatch() {
    while (!queue.empty()) {
        auto event = queue.front();
        queue.pop();
        God::camera.handle(event);
        God::layers.handle(event);
    }
}

