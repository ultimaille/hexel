#pragma once
#include "core.h"

struct RenderLayer {
    virtual ~RenderLayer() = default;
    virtual void render() = 0;
    virtual void generate_gui(std::string name) = 0;
//  virtual bool resync_with_data() = 0;
    virtual bool handle(Event event)=0;
    virtual void render_primitive_id()              { Log::add("To be implemented"); }
    virtual void render_constant_color(int layerid) { Log::add("To be implemented"); }
    // virtual bool is_cleanup_ready() { return false; }
    bool visible;
};

struct LayerManager: public Registry<RenderLayer> {
    void render() {
        for (auto& [name,obj] : *this)
            obj->render();
    }
    void handle(Event event) { // TODO: separate the cleanup logic from the update

        // //
        // if (event.event_type == Event::MM_REMOVED) {
        //     // Check if layer want to die
        //     for (auto& [name,obj] : *this) {
        //         if (obj->is_cleanup_ready()) {
        //             // obj->clean();
        //             erase(event.object_name);
        //         }
        //     }
        // }

        // clean up
        if (event.event_type == Event::RENDER_LAYER_REMOVED) {
            // int i = find(event.object_name);
            // operator[](i).clean();
            erase(event.object_name);
        }

        // dispatch events
        for (auto& [name,obj] : *this) {
            obj->handle(event);
        }
    }

    void produce_picking_image(int* data,int w,int h) { Log::add("To be implemented"); }

    #ifdef ACTIVATE
    // TODO move this in render target
    void read_framebuffer(std::vector<unsigned char> &data, vec4 rect, int attachment) {

        const auto [x, y, w, h] = std::tuple{rect[0], rect[1], rect[2], rect[3]};
        
        // Bind fbo
        glBindFramebuffer(GL_READ_FRAMEBUFFER, God::context.window.render_target.framebuffer);

        // Get convenient color attachment according to pick primitive id
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachment);

        data.resize(w * h * 4);

        // Read pixels
        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

        // Check for errors
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
            Log::error("Picking glReadPixels error: " + err);

        // Unbind fbo
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }
    #endif


};


