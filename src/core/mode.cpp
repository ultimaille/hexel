#include "mode.h"
#include "camera.h"
#include "core.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"

void MouseReactTrackBallCamera::on_wheel(double v){
    God::camera.zoom(v);
}
void MouseReactTrackBallCamera::on_drag(int button, vec2 a, vec2 b){
    auto [width, height] = God::context.screen_size();
    const vec2 viewport = {
        static_cast<double>(width),
        static_cast<double>(height)
    };
    if (button == 0) God::camera.pan(b - a, viewport);
    if (button == 1) God::camera.rotate(a, b, viewport);
}





void Mode::send_instruction_to_react(Event event,MouseReact& react) {

    static vec2 pos_on_press[3];

    if (event.who == events::MOUSE && !ImGui::GetIO().WantCaptureMouse) {
        const double wheel = God::mouse.wheel_speed;
        if (wheel != 0) react.on_wheel(wheel);
        vec2 a = { God::mouse.previous.x, God::mouse.previous.y };
        vec2 b = { God::mouse.current.x,  God::mouse.current.y };
        // send drag event
        if ((a - b).norm2() > 0) {
            FOR(button, 3)
                    if (God::mouse.down(button)) react.on_drag(button, a, b);
            //return;
        }
        // send pressed event
        FOR(button, 3) if (God::mouse.current.buttons[button] && !God::mouse.previous.buttons[button]) {
            pos_on_press[button] = b;
            react.on_press(button, b);
        }
        // send released and clicked event
        FOR(button, 3) if (!God::mouse.current.buttons[button] && God::mouse.previous.buttons[button]) {
                react.on_release(button, b);
                if ((b - pos_on_press[button]).norm2() == 0) {
                    react.on_click(button, b);
            }
        }
    }
}
