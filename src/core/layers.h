#pragma once
#include "core.h"

struct RenderLayer {
    RenderLayer() : _id(max_id) { ++max_id; }
    virtual ~RenderLayer() = default;
    virtual void render() = 0;
    virtual void generate_gui(std::string name) = 0;
    virtual bool handle(Event event) = 0;
    virtual void render_primitive_id()              { Log::add("To be implemented"); }
    virtual void render_constant_color(int layerid) { Log::add("To be implemented"); }
    // virtual bool is_cleanup_ready() { return false; }
    bool visible;

    int id() const {
        return _id;
    }

    protected:
    static inline int max_id = 0;
    int _id;
};

struct LayerManager: public Registry<RenderLayer> {
    void render() {
        for (auto& [name,obj] : *this)
            obj->render();
    }
    void handle(Event event) { // TODO: separate the cleanup logic from the update

        //// clean up
        //if (event.event_type == Event::RENDER_LAYER_REMOVED) {
        //    erase(event.object_name);
        //}

        // dispatch events
        for (auto& [name,obj] : *this) {
            obj->handle(event);
        }
    }

    void produce_picking_image(int* data,int w,int h) { Log::add("To be implemented"); }

};


