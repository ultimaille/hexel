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

void OrthographicCamera::update(){
    auto [w,h] = God::context.screen_size();
    resize(w, h);
    double wheel = God::mouse.get_wheel_event();
    if(wheel!=0) {
        plop(wheel);
        zoom *= (1.+.1*wheel);
        zoom = std::clamp(zoom,.2,5.);
    }
    if(!God::mouse.mouseDragging[1]) return;
    if(!God::keys.pressed(ImGuiKey_LeftCtrl)) return;

    rotX += .01*(God::mouse.y-God::mouse.lasty); rotX = std::clamp(rotX,-M_PI,M_PI);
    rotY -= .01*(God::mouse.x-God::mouse.lastx); rotY = std::clamp(rotY,-M_PI/2.,M_PI/2.);
}

#if 0
void TrackballCamera::update(){
	double wheel = God::mouse.get_wheel_event();
	if(wheel!=0) {
		plop(wheel);
		/*
		zoom_factor *= (1.+.1*wheel);
		zoom_factor = std::clamp(zoom_factor,.2,5.);
		*/
		zoom(wheel);
	}
	
	if(God::keys.pressed(ImGuiKey_LeftCtrl)) {
		if (God::mouse.mouseDragging[0]) {
			auto dx = God::mouse.x - God::mouse.lastx;
			auto dy = God::mouse.y - God::mouse.lasty;
			pan({dx, dy});
		} else if (God::mouse.mouseDragging[1]) {
			rotate({God::mouse.lastx, God::mouse.lasty}, {God::mouse.x, God::mouse.y});
		} 
	}
}
#endif
