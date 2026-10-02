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

        case LAYER:
            if (names.size() != 1) return std::nullopt;
            if (!God::layers.contains(names[0])) return std::nullopt;
            return std::ref(God::layers[names[0]]);

        /* TODO
            if (path.size() != 2) return std::nullopt;
            // Assuming LayerManager has a way to access layers by name
            if (auto layer = God::layers.get(names[0])) {
                return std::ref(*layer);
            }
            */
            return std::nullopt;

        case POINTSET: {
            if (names.size() != 1) return std::nullopt;
            std::string mm = names[0];
            if (!God::xcf.contains(mm)) return std::nullopt;
            return std::ref(God::xcf[mm].pointset);
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
/*


void* ObjectId::ptr() {
    um_assert(!chunks.empty());
    if (chunks[0]==chunk_mouse)
        return &God::mouse;
    if (chunks[0]==chunk_camera)
        return &God::camera;

    if (chunks[0]==chunk_xcf) {
        // return the xcf
        if (chunks.size()==1) return &God::xcf;

        // returns a multimesh
        um_assert(chunks.size() > 1);
        std::string mm = chunks[1];
        if (!God::xcf.contains(mm)) return nullptr;
        if (chunks.size() == 2)
            return &God::xcf[mm];

        // return a pointset
        um_assert(chunks.size() > 2);
        if (chunks[2] == chunk_pointset) return &God::xcf[mm].pointset;
        // TODO pointset_attributes

        // return a mesh+attributes
        um_assert(chunks.size() > 3);
        std::string mesh = chunks[3];
        if (chunks[2] == chunk_polylines) {
            if (!God::xcf[mm].polylines.contains(mesh)) return nullptr;
            return &God::xcf[mm].polylines[mesh];
        }
        if (chunks[2] == chunk_triangles) {
            if (!God::xcf[mm].triangles.contains(mesh)) return nullptr;
            return &God::xcf[mm].triangles[mesh];
        }
        if (chunks[2] == chunk_quads) {
            if (!God::xcf[mm].quads.contains(mesh)) return nullptr;
            return &God::xcf[mm].quads[mesh];
        }
    }
    um_assert(!"should not reach this point");
    return nullptr;
}
*/

void EventManager::dispatch() {
    while (!queue.empty()) {
        auto event = queue.front();
        queue.pop();
        God::camera.handle(event);
        God::layers.handle(event);
    }
}

ObjectId::operator PointSet&() {
    return as<PointSet>();
}

ObjectId::operator PointSetAttributes&() {
    return as<PointSetAttributes>();
}

ObjectId::operator Triangles&() {
    auto &ma = as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>();
    return ma.mesh;
}

ObjectId::operator SurfaceAttributes&() {
    switch (path) {
        case TRIANGLES: return as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>().attributes;
        case QUADS:     return as<MultiMesh::MeshAttr<Quads,     SurfaceAttributes>>().attributes;
        case POLYGONS:  return as<MultiMesh::MeshAttr<Polygons,  SurfaceAttributes>>().attributes;
        default: um_assert(!"invalid cast");
    }
}


/*


/*
ObjectId::operator PointSet&() {
    auto *p = static_cast<PointSet *>(ptr());
    um_assert(p != nullptr);
    return *p;
}

ObjectId::operator PointSetAttributes&() {
    auto *p = static_cast<PointSetAttributes *>(ptr());
    um_assert(p != nullptr);
    return *p;
}

ObjectId::operator PolyLine&() {
    auto *p = static_cast<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>*>(ptr());
    um_assert(p != nullptr);
    return p->mesh;
}

ObjectId::operator PolyLineAttributes&() {
    auto *p = static_cast<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>*>(ptr());
    um_assert(p != nullptr);
    return p->attributes;
}

*/
