#pragma once

struct Picker{

    struct PickResult {
        int x, y;
        int layer_id;
        int primitive_id;
        ObjectId object_id;
        float depth;

        // get_point
        // get_attr_value
        vec3 point();
    };

    std::vector<unsigned char> layer_ids;
    std::vector<unsigned char> vertex_ids;
    std::vector<float> depths;

    Picker();
    Picker(vec4 rect);

    void reset();

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

    PickResult at(vec2 uv);

    private:
    vec4 rect;
};

