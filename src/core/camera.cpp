#include "core.h"

#include "core.h"

void TrackBallCamera::update() {
    auto [width, height] = God::context.screen_size();
    const vec2 viewport = {
        static_cast<double>(width),
            static_cast<double>(height)
    };

    resize(viewport.x, viewport.y);

    const double wheel = God::mouse.get_wheel_event();
    if (wheel != 0) { // TODO add guards
        zoom(std::exp(-0.1 * wheel));
    }

    if (!God::keys.pressed(ImGuiKey_LeftCtrl))
        return;

    if (God::mouse.mouseDragging[0]) {
        const vec2 delta{
            God::mouse.x - God::mouse.lastx,
                God::mouse.y - God::mouse.lasty
        };

        pan(delta, viewport);
    } else if (God::mouse.mouseDragging[1])
        rotate({God::mouse.lastx, God::mouse.lasty}, {God::mouse.x, God::mouse.y}, viewport);
}


