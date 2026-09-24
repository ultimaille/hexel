#pragma once
#include "core.h"

struct RenderLayer {
    virtual ~RenderLayer() = default;
    virtual void render() = 0;
    virtual void generate_gui(std::string name) = 0;
    virtual bool resync_with_data() = 0;
    virtual void render_primitive_id()              { Log::add("To be implemented"); }
    virtual void render_constant_color(int layerid) { Log::add("To be implemented"); }
    bool visible;
};

struct LayerManager: public Registry<RenderLayer> {
    void render() {
        for (auto& [name,obj] : *this)
            obj->render();
    }
    void sync() { // TODO: separate the cleanup logic from the update
        FOR(i,size())
            if (!operator[](i).resync_with_data()) {
                std::swap(items[i],items.back());
                pop_back();
            }
    }
    void produce_picking_image(int* data,int w,int h) { Log::add("To be implemented"); }
};


