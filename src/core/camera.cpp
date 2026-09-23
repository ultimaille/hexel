#include "core.h"

void TrackBallCamera::update() {
    auto [width, height] = God::context.screen_size();
    const vec2 viewport = {
        static_cast<double>(width),
            static_cast<double>(height)
    };

    resize(viewport.x, viewport.y);

    const double wheel = God::mouse.get_wheel_event();
    if (wheel != 0)
        zoom(wheel);

    if (!God::keys.pressed(ImGuiKey_LeftCtrl))
        return;

    vec2 a = { God::mouse.lastx, God::mouse.lasty };
    vec2 b = { God::mouse.x,     God::mouse.y     };

    if (God::mouse.mouseDragging[0])
        pan(b-a, viewport);
    else if (God::mouse.mouseDragging[1])
        rotate(a, b, viewport);
}

