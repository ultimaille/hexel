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
                kill(i);
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

    void LayerManager::kill(int idx){
        std::string layer_name = ith_name(idx);

        // Clean GPU memory
        (*this)[idx].destroy();

        // Erase element
        int group_id = items[idx].object->group;
        items.erase(items.begin() + idx);

        // Eventually remove group
        if (God::layers.is_group_empty(group_id))
            God::layers.remove_group(group_id);

        // Broadcast event
        ObjectId(events::LAYER, layer_name).broadcast(events::KILLED);
    }

    void LayerManager::kill(std::string layer_name){
        int idx = find(layer_name);        
        kill(idx);
    }

    void LayerManager::destroy() {
        for (auto& [name, obj] : *this)
            obj->destroy();
    }
