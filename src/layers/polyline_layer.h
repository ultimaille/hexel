#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct PolyLineLayer : public Layer{
    SegmentRenderer primitive_renderer;
    PolyLineLayer();
    void generate_gui(std::string name);
    void init(ObjectId obj);
    void render();
    int primitive_type() { return 1; }// just for the version without attribute
    void destroy();
    void reset();
};