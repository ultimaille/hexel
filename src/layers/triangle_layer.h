#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>
#include <core/attribute.h>

struct TrianglesLayer : public Layer{
    TriangleRenderer primitive_renderer;
    TrianglesLayer();
    void generate_gui(std::string name);
    void reset() override;
    void init(ObjectId obj);
    void render();
    // void bind_attribute(ObjectId attr_id);
    void bind_attribute_qui_est_mieux(Attribute attr);
    void destroy();
};
