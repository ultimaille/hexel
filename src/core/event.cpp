#include "core.h"

void EventManager::dispatch() {
	while (!events.empty()) {
		auto event = events.back();
		events.pop_back();
		God::camera.on(event);
		// for (auto &l : God::layers)
		// 	l.on(event);
	}
}