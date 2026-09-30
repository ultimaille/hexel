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


void ObjectId::emit(EventType e) {
    God::events.queue.emplace(Event(*this, e));
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
        if (!God::xcf.contains(mm)) return NULL;
        if (chunks.size() == 2)
            return &God::xcf[mm];

        // return a pointset
        um_assert(chunks.size() > 2);
        if (chunks[2] == chunk_pointset) return &God::xcf[mm].pointset;

        // return a mesh+attributes
        um_assert(chunks.size() > 3);
        std::string mesh = chunks[3];
        if (chunks[2] == chunk_polylines) {
            if (!God::xcf[mm].polylines.contains(mesh)) return NULL;
            return &God::xcf[mm].polylines[mesh];
        }
        if (chunks[2] == chunk_triangles) {
            if (!God::xcf[mm].triangles.contains(mesh)) return NULL;
            return &God::xcf[mm].triangles[mesh];
        }
        if (chunks[2] == chunk_quads) {
            if (!God::xcf[mm].quads.contains(mesh)) return NULL;
            return &God::xcf[mm].quads[mesh];
        }
    }
    um_assert(!"should not reach this point");
    return NULL;
}


void EventManager::dispatch() {
    while (!queue.empty()) {
        auto event = queue.front();
        queue.pop();
        God::camera.handle(event);
        God::layers.handle(event);
    }
}

