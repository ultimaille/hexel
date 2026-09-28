#include "event.h"
#include "core.h"


    bool ObjectId::is_a(ObjectIdChunk e) {
        um_assert(!hardpath.empty());
        return hardpath.back() == e;
    }

    bool ObjectId::is(void* object) {
        um_assert(!hardpath.empty());
        if (is_a(mouse)) return static_cast<void*>(&God::mouse) == object;
        if (is_a(camera)) return static_cast<void*>(&God::camera) == object;

        if (hardpath[0] == xcf) {
            if (!God::xcf.contains(softpath[0])) return false;
            // return a multimesh
            if (hardpath.size() == 1) 
                return static_cast<void*>(&God::xcf[softpath[0]]) == object;
            
            // return a pointset or a combinatorial structure
            if (hardpath[1] == pointset) return static_cast<void*>(&God::xcf[softpath[0]].pointset) == object;
            um_assert(softpath.size()==2);
            if (hardpath[1] == polylines) {
                return static_cast<void*>(&God::xcf[softpath[0]].polylines[softpath[1]]) == object;
            }
            if (hardpath[1] == triangles) {
                if (!God::xcf[softpath[0]].triangles.contains(softpath[1])) return false;
                return static_cast<void*>(&God::xcf[softpath[0]].triangles[softpath[1]]) == object;
            }

            if (hardpath[1] == quads) return static_cast<void*>(&God::xcf[softpath[0]].quads[softpath[1]]) == object;
            if (hardpath[1] == polygons) return static_cast<void*>(&God::xcf[softpath[0]].polygons[softpath[1]]) == object;
            
            if (hardpath[1] == tetrahedra) return static_cast<void*>(&God::xcf[softpath[0]].tetrahedra[softpath[1]]) == object;
            if (hardpath[1] == hexahedra) return static_cast<void*>(&God::xcf[softpath[0]].hexahedra[softpath[1]]) == object;
            if (hardpath[1] == wedges) return static_cast<void*>(&God::xcf[softpath[0]].wedges[softpath[1]]) == object;
            if (hardpath[1] == pyramids) return static_cast<void*>(&God::xcf[softpath[0]].pyramids[softpath[1]]) == object;

        }
        return false;
    }


void EventManager::dispatch() {
	while (!events.empty()) {
		auto event = events.front();
		events.pop();
		God::camera.handle(event);
		God::layers.handle(event);
	}
    // clear input diff
    God::mouse.current_state.wheel_speed = 0;
    God::mouse.last_state = God::mouse.current_state;
}