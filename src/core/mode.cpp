#include "mode.h"
#include "camera.h"
#include "core.h"


    void MouseReactionTrackBallCamera::on_wheel(double v){
        dynamic_cast<TrackBallCamera&>(*God::camera.impl).zoom(v);
    }
    void MouseReactionTrackBallCamera::on_drag(int button, vec2 a, vec2 b){
        auto [width, height] = God::context.screen_size();
        const vec2 viewport = {
            static_cast<double>(width),
            static_cast<double>(height)
        };
        if (button == 0) dynamic_cast<TrackBallCamera&>(*God::camera.impl).pan(b - a, viewport);
        if (button == 1) dynamic_cast<TrackBallCamera&>(*God::camera.impl).rotate(a, b, viewport);
    }
