#include <array>
#include "core.h"

vec3 CameraPose::position() const { return pivot + orientation.rotate({0, 0, distance}); }
vec3 CameraPose::forward()  const { return orientation.rotate({0, 0, -1}); }
vec3 CameraPose::up()       const { return orientation.rotate({0, 1,  0}); }
vec3 CameraPose::right()    const { return orientation.rotate({1, 0,  0}); }

void CameraPose::rotate(Quaternion q) {
    orientation = q * orientation;
}

void CameraPose::translate(vec3 delta) {
    pivot += delta;
}

void CameraPose::pan(vec2 delta) {
    translate(right()*delta.x + up()*delta.y);
}

mat4x4 CameraPose::matrix() const {
    const mat3x3 R = orientation.rotation_matrix();
    const vec3   p = position();
    const mat4x4 T = {{
        {R[0][0], R[0][1], R[0][2], p.x},
        {R[1][0], R[1][1], R[1][2], p.y},
        {R[2][0], R[2][1], R[2][2], p.z},
        {0,       0,       0,       1  }
    }};
    return T.invert();
}


void OrthographicProjection::set_aspect_ratio(double a) {
    um_assert(a > 0);
    aspect = a;
}

mat4x4 OrthographicProjection::matrix() const {
    um_assert(aspect > 0);
    um_assert(view_height > 0);
    um_assert(far > near);

    const double view_width = view_height * aspect;

    // map [-view_width/2, view_width/2] x [-view_height/2, view_height/2] world rectangle to normalized device coordinates [-1,1]^2
    const double left   = center.x - view_width /2;
    const double right  = center.x + view_width /2;
    const double bottom = center.y - view_height/2;
    const double top    = center.y + view_height/2;
    return {{
        { 2 / (right - left), 0,                  0,                 -(right + left) / (right - left) },
        { 0,                  2 / (top - bottom), 0,                 -(top + bottom) / (top - bottom) },
        { 0,                  0,                  -2 / (far - near), -(far + near) / (far - near)     },
        { 0,                  0,                  0,                 1                                }
    }};
}

Quaternion TrackBallCamera::arcball_rotation(vec2 previous, vec2 current, vec2 viewport) const {
    // map a screen position to the virtual trackball
    const auto arcball_point = [](vec2 p, vec2 viewport) -> vec3 {
        //std::cerr << p << std::endl;
        // the virtual sphere has radius 1 and is centered in the viewport
        // the smaller viewport dimension is used so that the sphere remains circular
        const auto [w, h] = viewport;
        const double size = std::min(w, h);

        // screen coordinates:
        //  (-1,+1) -------- (+1,+1)
        //     |                |
        //     |        0       |
        //     |                |
        //  (-1,-1) -------- (+1,-1)

        const double x = (2*p.x - w)/size;
        const double y = (h - 2*p.y)/size;
        const double r2 = x*x + y*y;

        if (r2 <= 1) // inside
            return {x, y, std::sqrt(1 - r2)};

        // outside the sphere, project onto the equator
        const double r = std::sqrt(r2);
        return { x/r, y/r, 0 };
    };

    vec3 v0 = arcball_point(previous, viewport); // both points lie on the unit sphere,
    vec3 v1 = arcball_point(current,  viewport); // so their dot product is cos(theta)
    return Quaternion::shortest_rotation(v1, v0);
}


void TrackBallCamera::resize(double width, double height) {
    um_assert(width  > 0);
    um_assert(height > 0);
    projection.set_aspect_ratio(width/height);
}

void TrackBallCamera::zoom(double wheel) {
    projection.view_height = std::max(
            min_view_height,
            projection.view_height * std::exp(-wheel_zoom_speed * wheel) // mouse input controls the logarithm of the camera scale
            );
}

void TrackBallCamera::pan(vec2 delta, vec2 viewport) {
    um_assert(viewport.x > 0);
    um_assert(viewport.y > 0);
    const double world_units_per_pixel = projection.view_height / viewport.y;
    pose.pan({ -delta.x * world_units_per_pixel, delta.y * world_units_per_pixel });
}

void TrackBallCamera::rotate(vec2 previous, vec2 current, vec2 viewport) {
    Quaternion q = arcball_rotation(previous, current, viewport);
    pose.orientation = pose.orientation * q;
}

mat4x4 TrackBallCamera::projection_matrix() const {
    return projection.matrix();
}

mat4x4 TrackBallCamera::view_matrix() const {
    return pose.matrix();
}

void TrackBallCamera::handle(Event event) {
    auto [width, height] = God::context.screen_size();
    const vec2 viewport = {
        static_cast<double>(width),
        static_cast<double>(height)
    };

    resize(viewport.x, viewport.y);

    if (event.event_type == Event::MOUSE_SCROLLED) {
        const double wheel = God::mouse.wheel_event_speed;
        if (wheel != 0)
            zoom(wheel);
    } else if (event.event_type == Event::MOUSE_MOVED) {
        if (!God::keys.pressed(GLFW_KEY_LEFT_CONTROL))
            return;

        vec2 a = { God::mouse.lastx, God::mouse.lasty };
        vec2 b = { God::mouse.x,     God::mouse.y     };

        if (God::mouse.mouseDragging[0])
            pan(b-a, viewport);
        else if (God::mouse.mouseDragging[1])
            rotate(a, b, viewport);
    }
}

float *row_major(const mat4x4 &m) {
    static thread_local std::array<float, 16> result; // thread-local storage keeps the returned pointer valid after return.
    for (int i = 0; i<16; ++i)
        result[i] = static_cast<float>(m[i/4][i%4]);
    return result.data();
}

float* Camera::projection() const {
    mat4x4 m = impl->projection_matrix();
    return row_major(m);
}

float* Camera::view() const {
    mat4x4 m = impl->view_matrix();
    return row_major(m);
}

float* Camera::inverse_projection() const {
    mat4x4 m = impl->projection_matrix().invert();
    return row_major(m);
}

void Camera::handle(Event event) {
    impl->handle(event);
}

