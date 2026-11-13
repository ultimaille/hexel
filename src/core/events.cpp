#include "events.h"
#include "core.h"
#include "camera.h"
#include "layers.h"
#include "mode.h"

void EventManager::dispatch() {
    while (!queue.empty()) {
        auto event = queue.back();
        queue.pop_back();
        God::camera.handle(event);
        God::layers.handle(event);
        God::modes.handle(event);
    }
}

