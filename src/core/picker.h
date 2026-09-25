#pragma once

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
        const int x = static_cast<int>(uv.x);
        const int y = static_cast<int>(uv.y);
        const int w = static_cast<int>(rect[2]);
        const int h = static_cast<int>(rect[3]);
        // Vérification des bornes pour éviter tout crash hors image
        if (x < 0 || x >= w || y < 0 || y >= h) {
            return {-1, -1};
        }

        // Inversion de l'axe Y : l'origine OpenGL est en bas à gauche
        int flipped_y = h - 1 - y;

        int off = (flipped_y * w + x) * 4;
        int layer_id = decode(layer_data, off);
        int primitive_id = decode(primitive_data, off);
        return {layer_id, primitive_id};
    }

    private:
    vec4 rect;
};

