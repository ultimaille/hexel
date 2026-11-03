#pragma once
#include <ultimaille/all.h>
using namespace UM;
//#include <algorithm>
//#include <cmath>

struct iter_raster_line {
    vec2 a, b;
    struct iterator {
        int x, y, ex, ey, dx, dy, sx, sy, err;
        iterator(vec2 a, vec2 b);
        vec2 operator*() const;
        iterator& operator++();
        bool operator!=(const iterator& o) const;
    };
    iterator begin() const;
    iterator end()   const;
};






struct iter_raster_triangle {
    Triangle2 tri;
    struct iterator {
        BBox2 bbox;
        vec2 p;
        Triangle2 tri;

        iterator(Triangle2 tri, bool end = false);

        void skip();

        vec2 operator*() const;

        iterator& operator++();

        bool operator!=(const iterator& o) const;
    };

    iterator begin() const;
    iterator end()   const;
};


struct iter_raster_polygon {
    
    iter_raster_polygon(std::vector<vec2>& pts) ;
    std::vector<vec2>& pts;

    struct iterator {
        BBox2 bbox;
        vec2 p;
        std::vector<vec2>& pts;

        iterator(std::vector<vec2>& pts, bool end = false);


        void skip();

        vec2 operator*() const;

        iterator& operator++();

        bool operator!=(const iterator& o) const;
    };

    iterator begin() const;
    iterator end()   const;
};

