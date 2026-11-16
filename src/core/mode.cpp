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


bool Interactor::key_state_is_OK(){
    if (ctrl_must_be_down && !God::keys.pressed(GLFW_KEY_LEFT_CONTROL)) return false;
    if (shift_must_be_down && !God::keys.pressed(GLFW_KEY_LEFT_SHIFT)) return false;
    if (ctrl_must_be_up && God::keys.pressed(GLFW_KEY_LEFT_CONTROL)) return false;
    if (shift_must_be_up && God::keys.pressed(GLFW_KEY_LEFT_SHIFT)) return false;
    return true;
}






void InteractorTrackBallCamera::on_wheel(double v){
    if (!key_state_is_OK()) return;
    God::camera.zoom(-v);
}
void InteractorTrackBallCamera::on_drag(int button, vec2 a, vec2 b){
    if (!key_state_is_OK()) return;
    auto [width, height] = God::context.screen_size();
    vec2 viewport (width,height);
    if (button == 0) God::camera.pan(b - a, viewport);
    if (button == 1) God::camera.rotate(a, b, viewport);
}


void Mode::draw_data_gui(){
    for (auto& it : interactors)
        it->draw_data_gui();
}
void Mode::generate_interactor_gui(){
    for (auto& it : interactors)
        it->generate_gui();
}

void Mode::handle(Event event){

    for (auto& it : interactors){
        if (event.who == root.mouse) {
            it->handle(event);
            continue;
        }
    }

    static vec2 pos_on_press[3];

    if (event.who == root.mouse && !ImGui::GetIO().WantCaptureMouse) {
        static int npixels_tolerance_to_detect_click = 5;
        const double wheel = God::mouse.wheel_speed;

        if (wheel != 0) for (auto& it : interactors) it->on_wheel(wheel);
        vec2 a = { God::mouse.previous.x, God::mouse.previous.y };
        vec2 b = { God::mouse.current.x,  God::mouse.current.y };
        // send drag event
        if ((a - b).norm2() > 0) {
            FOR(button, 3)
                if (God::mouse.down(button)) for (auto& it : interactors) it->on_drag(button, a, b);
        }
        // send pressed event
        FOR(button, 3) if (God::mouse.current.buttons[button] && !God::mouse.previous.buttons[button]) {
            pos_on_press[button] = b;
            for (auto& it : interactors) it->on_press(button, b);
        }
        // send released and clicked event
        FOR(button, 3) if (!God::mouse.current.buttons[button] && God::mouse.previous.buttons[button]) {
            for (auto& it : interactors) it->on_release(button, b);
            if ((b - pos_on_press[button]).norm2() < npixels_tolerance_to_detect_click * npixels_tolerance_to_detect_click)
                for (auto& it : interactors) it->on_click(button, b);
        }
    }

}


DefaultMode::DefaultMode(){
    interactors.push_back(&interactor_camera);
}
