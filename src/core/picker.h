#pragma once

struct Picker{

    std::vector<unsigned char> layer_ids;
    std::vector<unsigned char> vertex_ids;

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

    std::tuple<int, int, ObjectId> at(vec2 uv);

    private:
    vec4 rect;
};

