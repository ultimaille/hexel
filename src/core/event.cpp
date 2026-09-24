#include "core.h"

void EventManager::dispatch() {
	while (!events.empty()) {
		auto event = events.front();
		events.pop();
		God::camera.handle(event);
		// for (auto &l : God::layers)
		// 	l.handle(event);
	}
}