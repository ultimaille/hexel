#pragma once

struct Picker{

    
    struct Result {
        int x, y;
        int layer_id;
        int raw_primitive_id;
        MeshPath mesh_id;
        float depth;
        int buffer_idx;
        vec3 point();
        // in most cases, we're interested to know if the picked point is on a specific primitive/mesh pair
        int filtered_primitive_id(MeshPath requested_mesh, AttributeBase::TYPE requested_primitive_type);
    };

    struct BufferData {
        friend class Picker;

        BufferData(int width, int height, int buf) : w(width), h(height), buffer_idx(buf) {}
        int w, h;
        int buffer_idx;
        std::vector<unsigned char> layer_ids;
        std::vector<unsigned char> primitive_ids;
        std::vector<float> depths;

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
    private:
        Result at(vec2 uv);
    };


    Result at(vec2 uv, int buffer);
    int nbuffers();
    Picker();
    void reset();
private:
    std::vector<BufferData> data;
    int w, h;
    bool need_reset = true;
    void jit_reset();
};

