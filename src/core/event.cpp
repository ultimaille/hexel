#include "event.h"
#include "core.h"


const std::string chunk_xcf = "xcf";
const std::string chunk_pointset = "pointset";
const std::string chunk_polylines = "polylines";
const std::string chunk_triangles = "triangles";
const std::string chunk_quads = "quads";
const std::string chunk_polygons = "polygons";
const std::string chunk_tetrahedra = "tetrahedra";
const std::string chunk_hexahedra = "hexahedra";
const std::string chunk_wedges = "wedges";
const std::string chunk_pyramids = "pyramids";
const std::string chunk_layer = "layer";
const std::string chunk_camera = "camera";
const std::string chunk_mouse = "mouse";
const std::string chunk_key = "key";
const std::string chunk_panel = "panel";

void ObjectId::emit(EventType e) {
    God::events.queue.emplace(Event(*this, e));
}


void* ObjectId::ptr() {
    um_assert(!chunks.empty());
    if (chunks[0] == chunk_mouse)
        return &God::mouse;
    if (chunks[0]==chunk_camera)
        return &God::camera;
    if (chunks[0] == chunk_panel){
        if (chunks.size() == 1)
            return &God::panels;
        if (chunks.size() == 2){
            if (!God::panels.contains(chunks[1])) return nullptr;
            return &God::panels[chunks[1]];
        }
    }
    if (chunks[0] == chunk_layer){
        if (chunks.size() == 1)
            return &God::layers;
        if (chunks.size() == 2){
            if (!God::layers.contains(chunks[1])) return nullptr;
            return &God::layers[chunks[1]];
        }
    }

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

ObjectId::operator Panel& () {
    auto* p = static_cast<Panel*>(ptr());
    um_assert(p != nullptr);
    return *p;
}

ObjectId::operator RenderLayer& () {
    auto* p = static_cast<RenderLayer*>(ptr());
    um_assert(p != nullptr);
    return *p;
}

ObjectId::operator Quads&() {
#if 1
    auto *p = static_cast<MultiMesh::MeshAttr<Quads, SurfaceAttributes>*>(ptr());
    um_assert(p != nullptr);
    return p->mesh;
#else
    um_assert(chunks.size() == 4);
    um_assert(chunks[0] == chunk_xcf);
    std::string mm = chunks[1];
    um_assert(God::xcf.contains(mm));
    um_assert(chunks[2] == chunk_quads);
    std::string mesh = chunks[3];
    um_assert(God::xcf[mm].quads.contains(mesh));
    return God::xcf[mm].quads[mesh].mesh;
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


