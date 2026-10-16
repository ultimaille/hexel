#pragma once

struct Picker{

    struct Result {
        int x, y;
        int layer_id;
        int primitive_id;
        ObjectId mesh_id;
        float depth;
        // get_attr_value
        vec3 point();
        // in most cases, we're interested to know if the picked point is on a specific primitive/mesh pair
        int get_primitive_id(ObjectId& requested_mesh, int requested_primitive_type);
    };

    std::vector<unsigned char> layer_ids;
    std::vector<unsigned char> primitive_ids;
    std::vector<float> depths;

    Picker();

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

    Result at(vec2 uv);

    private:
    int w, h;
};

