#include "raster_iterator.h"
#define _USE_MATH_DEFINES 
#include <math.h>

//=============LINE=======================================================================
iter_raster_line::iterator::iterator(vec2 a, vec2 b)
    : x(std::round(a.x)), y(std::round(a.y)),
    ex(std::round(b.x)), ey(std::round(b.y)),
    dx(std::abs(ex - x)), dy(std::abs(ey - y)),
    sx(x < ex ? 1 : -1), sy(y < ey ? 1 : -1),
    err(dx - dy) {}
vec2 iter_raster_line::iterator::operator*() const { return { double(x), double(y) }; }
iter_raster_line::iterator& iter_raster_line::iterator::operator++() {
    int e = 2 * err;
    if (e > -dy) err -= dy, x += sx;
    if (e < dx) err += dx, y += sy;
    return *this;
}
bool iter_raster_line::iterator::operator!=(const iterator& o) const {
    return x != o.x || y != o.y;
}
iter_raster_line::iterator iter_raster_line::begin() const { return { a, b }; }
iter_raster_line::iterator iter_raster_line::end()   const { return { b, b }; }



//=============TRIANGLE====================================================================
iter_raster_triangle::iterator::iterator(Triangle2 tri, bool end) : tri(tri) {
    for (int lv = 0; lv < 3; lv++) bbox.add(tri[lv]);
    for (int d = 0; d < 2; d++) {
        bbox.min[d] = std::ceil(bbox.min[d]);
        bbox.max[d] = std::floor(bbox.max[d]);
    }
    p = bbox.min;
    if (end) p.y = bbox.max.y + 1;
    if (!end) skip();
}
void iter_raster_triangle::iterator::skip() {
    while (p.y <= bbox.max.y) {
        while (p.x <= bbox.max.x) {
            vec3 c = tri.bary_coords(p);
            bool in = true;
            for (int d = 0; d < 3; d++) if (c[d] > 1 || c[d] < 0) in = false;
            if (in) return;
            p.x += 1.;
        }
        p.x = bbox.min[0];
        p.y += 1;
    }
}
vec2 iter_raster_triangle::iterator::operator*() const { return p; }
iter_raster_triangle::iterator& iter_raster_triangle::iterator::operator++() {
    um_assert(p.x <= bbox.max.x);
    p.x += 1.;
    skip();
    return *this;
}
bool iter_raster_triangle::iterator::operator!=(const iterator& o) const {
    return (p - o.p).norm2() > 0;
}
iter_raster_triangle::iterator iter_raster_triangle::begin() const { return { tri }; }
iter_raster_triangle::iterator iter_raster_triangle::end()   const { return { tri, true }; }



//=============POLYGON====================================================================
iter_raster_polygon::iter_raster_polygon(std::vector<vec2>& pts) :pts(pts) {};
iter_raster_polygon::iterator::iterator(std::vector<vec2>& pts, bool end) : pts(pts) {
    for (vec2 pt : pts) bbox.add(pt);
    for (int d = 0; d < 2; d++) {
        bbox.min[d] = std::ceil(bbox.min[d]);
        bbox.max[d] = std::floor(bbox.max[d]);
    }
    p = bbox.min;
    if (end) p.y = bbox.max.y + 1;
    if (!end) skip();
}
void iter_raster_polygon::iterator::skip() {
    while (p.y <= bbox.max.y) {
        while (p.x <= bbox.max.x) {
            double angle = 0;
            for (int i = 0; i < pts.size(); i++) {
                vec2 a = pts[i];
                vec2 b = pts[(i + 1) % pts.size()];
                if ((a - p).norm2() < 1e-10) return;        // points on a vertex are considered in
                if ((b - p).norm2() < 1e-10) return;        // points on a vertex are considered in
                vec2 u = (a - p).normalized();
                vec2 v = (b - p).normalized();
                double dangle = std::atan2(u.x * v.y - u.y * v.x, u * v);
                if (dangle < -.99 * M_PI) dangle = M_PI;    // points on segment are considered in w.r.t the segment
                angle += dangle;
            }
            if (std::round(angle / (2. * M_PI)) == 1) return;
            p.x += 1.;
        }
        p.x = bbox.min[0];
        p.y += 1;
    }
}
vec2 iter_raster_polygon::iterator::operator*() const { return p; }
iter_raster_polygon::iterator& iter_raster_polygon::iterator::operator++() {
    um_assert(p.x <= bbox.max.x);
    p.x += 1.;
    skip();
    return *this;
}
bool iter_raster_polygon::iterator::operator!=(const iterator& o) const {
    return (p - o.p).norm2() > 0;
}
iter_raster_polygon::iterator iter_raster_polygon::begin() const { return { pts }; }
iter_raster_polygon::iterator iter_raster_polygon::end()   const { return { pts, true }; }

