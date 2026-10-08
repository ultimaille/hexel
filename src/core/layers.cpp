#include "layers.h"

#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "shaders.h"
#include "mode.h"


    Layer::Layer() : id(max_id) { ++max_id; }
    void Layer::reset() { Log::error("reset called for a layer that does not implement it"); };
    void Layer::generate_gui(std::string name) {
        ImGui::Checkbox(("visible##visible" + name).c_str(), &visible);
    }


    void Layer::render_primitive_id()              { Log::add("To be implemented"); }
    void Layer::render_constant_color(int layerid) { Log::add("To be implemented"); }
    void Layer::destroy() {}


    int Layer::primitive_id(int vertex_id) { return vertex_id; } // TODO to pure virtual






    void LayerManager::render() {
        for (auto& [name, obj] : *this)
            if (obj->visible)
                obj->render();
    }
    void LayerManager::handle(Event event) {
        // if (event.who == events::MOUSE) return;


        // manage lifecycle events (KILLED MESH)
        std::vector<std::string> to_kill;
        if (event.what_happened == events::KILLED) for (int i = 0; i < this->size(); i) {
            if ((*this)[i].mesh==event.who)
                erase(i);
            else i++;
        }
        if (event.what_happened == events::UPDATED)
            for (auto& shad : items)
                if (shad.object->mesh==event.who)
                    shad.object->reset();
    }



    std::optional<std::reference_wrapper<Layer>> LayerManager::find_by_id(int id) {
        for (auto& [name, obj] : *this) {
            if (obj->id == id)
                return *obj;
        }
        return std::nullopt;
    }

    void LayerManager::kill(std::string layer_name){
        (*this)[layer_name].destroy();
        int id = find(layer_name);
        std::swap(items[id], items.back());
        items.pop_back();
        ObjectId(events::LAYER, layer_name).broadcast(events::KILLED);
    }

    void LayerManager::destroy() {
        for (auto& [name, obj] : *this)
            obj->destroy();
    }
