#include "camera.h"
#include "core.h"

void OrthographicCamera::update(){
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

