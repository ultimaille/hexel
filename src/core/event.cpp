#include "event.h"
#include "core.h"


void ObjectId::emit(EventType e) {
    God::events.queue.emplace(Event(*this, e));
}

std::optional<ObjectId::ObjectPtr> ObjectId::ptr() {
    switch (path) {
        case Type::KEYBOARD:
            return std::ref(God::keys);

        case Type::MOUSE:
            return std::ref(God::mouse);

        case Type::CAMERA:
            return std::ref(God::camera);

        case Type::LAYER:
        /* TODO
            if (path.size() != 2) return std::nullopt;
            // Assuming LayerManager has a way to access layers by name
            if (auto layer = God::layers.get(names[0])) {
                return std::ref(*layer);
            }
            */
            return std::nullopt;

        case Type::XCF: {
            if (names.size() != 2) return std::nullopt;
            std::string mm = names[0];
            std::string m  = names[1];
            if (!God::xcf.contains(mm) ) return std::nullopt;

/*
            auto& multimesh = God::xcf[mm_name];

            // Return the multimesh itself (if depth == 2)
            if (path.size() == 2) {
                // TODO: How to return the multimesh container?
                return std::nullopt;
            }

            // Navigate deeper into mesh
            if (path.size() < 4) return std::nullopt;

            Type mesh_type = path[1];
            std::string mesh_name = names[1];

            switch (mesh_type) {
                case Type::POINTSET:
                    if (path.size() == 3) return std::ref(multimesh.pointset);
                    if (path.size() == 4 && path[3] == Type::POINTSET) {
                        // Accessing pointset attributes
                        return std::ref(multimesh.pointset); // TODO: return attributes
                    }
                    return std::nullopt;

                case Type::POLYLINES:
                    if (!multimesh.polylines.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.polylines[mesh_name].mesh);

                case Type::TRIANGLES:
                    if (!multimesh.triangles.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.triangles[mesh_name].mesh);

                case Type::QUADS:
                    if (!multimesh.quads.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.quads[mesh_name].mesh);

                case Type::POLYGONS:
                    if (!multimesh.polygons.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.polygons[mesh_name].mesh);

                case Type::TETRAHEDRA:
                    if (!multimesh.tetrahedra.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.tetrahedra[mesh_name].mesh);

                case Type::HEXAHEDRA:
                    if (!multimesh.hexahedra.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.hexahedra[mesh_name].mesh);

                case Type::WEDGES:
                    if (!multimesh.wedges.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.wedges[mesh_name].mesh);

                case Type::PYRAMIDS:
                    if (!multimesh.pyramids.contains(mesh_name)) return std::nullopt;
                    return std::ref(multimesh.pyramids[mesh_name].mesh);
                default:
                    return std::nullopt;
            }
*/
        }

        default:
            return std::nullopt;
    }
}


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


void EventManager::dispatch() {
    while (!queue.empty()) {
        auto event = queue.front();
        queue.pop();
        God::camera.handle(event);
        God::layers.handle(event);
    }
}

ObjectId::operator Triangles&() {
#if 1
    auto *p = static_cast<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>*>(ptr());
    um_assert(p != nullptr);
    return p->mesh;
#else
    um_assert(chunks.size() == 4);
    um_assert(chunks[0] == chunk_xcf);
    std::string mm = chunks[1];
    um_assert(God::xcf.contains(mm));
    um_assert(chunks[2] == chunk_triangles);
    std::string mesh = chunks[3];
    um_assert(God::xcf[mm].triangles.contains(mesh));
    return God::xcf[mm].triangles[mesh].mesh;
#endif
}

ObjectId::operator SurfaceAttributes&() {
    auto *p = static_cast<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>*>(ptr());
    um_assert(p != nullptr);
    return p->attributes;
}

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


