#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct RenderLambertTriangles : public RenderLayer{
    TriangleRenderer primitive_renderer;

    RenderLambertTriangles();

    void generate_gui(std::string name);

    bool handle(Event event);

    void reset();

    void init(ObjectId obj);

    void render();

    virtual int primitive_id(int vertex_id);

    void destroy();
};

struct RenderLambertQuads : public RenderLayer {
    TriangleRenderer primitive_renderer;
    RenderLambertQuads();
    void generate_gui(std::string name);
    bool handle(Event event);
    void reset();
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};


struct RenderLambertTet : public RenderLayer {
    TriangleRenderer primitive_renderer;
    RenderLambertTet();
    void generate_gui(std::string name);
    bool handle(Event event);
    void reset();
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};

struct RenderLambertHex : public RenderLayer {
    TriangleRenderer primitive_renderer;
    RenderLambertHex();
    void generate_gui(std::string name);
    bool handle(Event event);
    void reset();
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};

struct RenderSpheres : public RenderLayer{
    PointRenderer primitive_renderer;
    RenderSpheres();
    void generate_gui(std::string name);
    bool handle(Event event);
    void init(ObjectId obj);
    void render();
    void destroy();
};

struct RenderTubes : public RenderLayer{
    SegmentRenderer primitive_renderer;
    RenderTubes();
    void generate_gui(std::string name);
    bool handle(Event event);
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
};








