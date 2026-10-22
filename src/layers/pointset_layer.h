#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct PointSetLayer : public Layer{
    PointRenderer primitive_renderer;
    PointSetLayer();
    void generate_gui(std::string name);
    void init(ObjectId obj);
    void render();
    void destroy();
    void reset();
};
