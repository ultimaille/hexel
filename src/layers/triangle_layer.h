#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct TrianglesLayer : public Layer{
    TriangleRenderer primitive_renderer;
    TrianglesLayer();
    void generate_gui(std::string name);
    void reset() override;
    void init(ObjectId obj);
    void render();
    // void bind_attribute(ObjectId attr_id);
    void destroy();
};
