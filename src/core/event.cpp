#include "core.h"

void EventManager::dispatch() {
	while (!events.empty()) {
		auto event = events.front();
		events.pop();
		God::camera.handle(event);
		God::layers.handle(event);
	}
}