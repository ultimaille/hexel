#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>
#include <layers/hexahedra_layer.h>

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

struct PointSetLayer : public Layer{
    PointRenderer primitive_renderer;
    PointSetLayer();
    void generate_gui(std::string name);
    void init(ObjectId obj);
    void render();
    void destroy();
    void reset();
};

struct PolyLineLayer : public Layer{
    SegmentRenderer primitive_renderer;
    PolyLineLayer();
    void generate_gui(std::string name);
    void init(ObjectId obj);
    void render();
    int primitive_type() { return 1; }// just for the version without attribute
    virtual int primitive_id(int vertex_id);
    void destroy();
    void reset();
};








