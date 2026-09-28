#include "event.h"
#include "core.h"


    bool EventId::is_a(EventIdChunk e) {
        um_assert(!hardpath.empty());
        return hardpath.back() == e;
    }

    bool EventId::is(void* object) {
        um_assert(!hardpath.empty());
        if (is_a(mouse)) return static_cast<void*>(&God::mouse) == object;
        if (is_a(camera)) return static_cast<void*>(&God::camera) == object;

        if (hardpath[0] == xcf) {
            if (hardpath.size() == 1) {// return a multimesh
                return static_cast<void*>(&God::xcf[softpath[0]]) == object;
            }
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