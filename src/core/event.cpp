#include "event.h"
#include "layers.h"
#include "camera.h"
#include "core.h"
#include "mode.h"

void EventManager::dispatch() {
    while (!queue.empty()) {
        auto event = queue.front();
        queue.pop();
        God::camera.handle(event);
        God::layers.handle(event);
        God::modes.handle(event);
    }
}

