#pragma once
#include "core.h"
#include <optional>

struct RenderLayer {
    RenderLayer() : _id(max_id) { ++max_id; }
    virtual ~RenderLayer() = default;
    virtual void render() = 0;
    virtual void reset() { Log::error("reset called for a layer that does not implement it"); };
    virtual void generate_gui(std::string name) {
        if (ImGui::TreeNode((name).c_str())) 
            ImGui::TreePop();
    }
    
    virtual bool handle(Event event) = 0;
    virtual bool require(ObjectId object) = 0;

    virtual void render_primitive_id()              { Log::add("To be implemented"); }
    virtual void render_constant_color(int layerid) { Log::add("To be implemented"); }
    bool visible =true;

    int id() const {
        return _id;
    }

    virtual int primitive_id(int vertex_id) { return vertex_id; } // TODO to pure virtual

    protected:
    static inline int max_id = 0;
    int _id;
};

struct LayerManager: public Registry<RenderLayer> {
    void render() {
        for (auto& [name,obj] : *this)
            obj->render();
    }
    void handle(Event event) { 
        if (event.who == ObjectId({ chunk_mouse })) return;

        // dispatch events
        for (auto& [name,obj] : *this) obj->handle(event);


        // manage lifecycle events (KILLED MESH)
        std::vector<std::string> to_kill;
        if (event.what_happened == KILLED) for (int i = 0; i < this->size(); i) {
            if ((*this)[i].require(event.who))
                erase(i);
            else i++;
        }
        if (event.what_happened == UPDATED) 
            for(auto& shad:items)
                if (shad.object->require(event.who)) 
                    shad.object->reset();
    }

    void produce_picking_image(int* data,int w,int h) { Log::add("To be implemented"); }

    std::optional<std::reference_wrapper<RenderLayer>> find_by_id(int id) {
        for (auto &[name, obj] : *this) {
            if (obj->id())
                return *obj;
        }
        return std::nullopt;
    }
};


