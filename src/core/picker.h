#pragma once
#include "core.h"

struct Picker{

    std::vector<unsigned char> layer_data;
    std::vector<unsigned char> primitive_data;

    Picker();
    Picker(vec4 rect);

    int decode(std::vector<unsigned char> &data, int off) {
        unsigned char r = data[off];
        unsigned char g = data[off + 1];
        unsigned char b = data[off + 2];
        unsigned char a = data[off + 3];

        return a == 0 ? -1 :
                    r +
                    g * 256 +
                    b * 256 * 256;
    }

    std::tuple<int,int> at(vec2 uv) {
        int off = (uv.y * w + uv.x) * 4;
        int layer_id = decode(layer_data, off);
        int primitive_id = decode(primitive_data, off);
        return {layer_id, primitive_id};
    }

    private:
    int w = 0;
    int h = 0;
};

