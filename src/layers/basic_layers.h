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

struct QuadsLayer : public Layer {
    TriangleRenderer primitive_renderer;
    QuadsLayer();
    void generate_gui(std::string name);
    void reset();
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};


struct TetrahedraLayer : public Layer {
    TriangleRenderer primitive_renderer;
    TetrahedraLayer();
    void generate_gui(std::string name);
    void reset();
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};

struct HexahedraLayer : public Layer {
    TriangleRenderer primitive_renderer;
    float shrink = 0;
    HexahedraLayer();
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
};

struct PolyLineLayer : public Layer{
    SegmentRenderer primitive_renderer;
    PolyLineLayer();
    void generate_gui(std::string name);
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};








