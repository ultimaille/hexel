#pragma once
#include "core.h"
#include <optional>
#include <core/event.h>

struct RenderLayer {
    RenderLayer();
    virtual ~RenderLayer() = default;
    virtual void render() = 0;
    virtual void reset();
    virtual void generate_gui(std::string name);

    virtual bool handle(Event event) = 0;
    virtual bool require(ObjectId object);
    ObjectId& mesh();

    virtual void render_primitive_id();
    virtual void render_constant_color(int layerid);
    virtual void destroy();

    int id() const; //?!? pourquoi un accesseur ?

    virtual int primitive_id(int vertex_id);

    bool visible = true;
    ObjectId _mesh;
    static inline int max_id = 0;
    int _id;
};

struct LayerManager: private Registry<RenderLayer> {
    int size()                                 { return Registry<RenderLayer>::size(); }
    RenderLayer& operator[](int i)             { return Registry<RenderLayer>::operator[](i); }
    RenderLayer& operator[](std::string s)     { return Registry<RenderLayer>::operator[](s); }
    bool contains(std::string s)               { return Registry<RenderLayer>::contains(s); }
    int find(std::string str)                  { return Registry<RenderLayer>::find(str); }
    std::string ith_name(int i)                { return items[i].name; }
    void swap(int i, int j)                    { std::swap(items[i], items[j]); }
    template<class T> T& add(std::string str)  { return emplace_back<T>(str); }

    void render();
    void handle(Event event);
    std::optional<std::reference_wrapper<RenderLayer>> find_by_id(int id);
    void kill(std::string layer_name);

    void destroy();
};


