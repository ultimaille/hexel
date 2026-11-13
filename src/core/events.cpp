#include "events.h"
#include "core.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"
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

