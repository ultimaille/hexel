#pragma once
#include "core.h"

struct RenderLayer {
    virtual ~RenderLayer() = default;
    virtual void render() = 0;
    virtual void generate_gui(std::string name) = 0;
    virtual bool handle(Event event)=0;
    virtual void render_primitive_id()              { Log::add("To be implemented"); }
    virtual void render_constant_color(int layerid) { Log::add("To be implemented"); }
    bool visible;
};

struct LayerManager: public Registry<RenderLayer> {
    void render() {
        for (auto& [name,obj] : *this)
            obj->render();
    }
    void handle(Event event) { // TODO: separate the cleanup logic from the update

        // clean up
        if (event.even_type == Event::RENDER_LAYER_REMOVED) {
            // int i = find(event.object_name);
            // operator[](i).clean();
            erase(event.object_name);
        }

        // dispatch events
        for (auto& [name,obj] : *this)
            obj->handle(event);
    }
    void produce_picking_image(int* data,int w,int h) { Log::add("To be implemented"); }
};


