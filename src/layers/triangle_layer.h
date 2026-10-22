#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct TrianglesLayer : public Layer{
    TriangleRenderer primitive_renderer;
    TrianglesLayer();
    void generate_gui(std::string name);
    void reset();
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};
