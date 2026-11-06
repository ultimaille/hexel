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
        // ImGui::Checkbox(("visible##visible" + name).c_str(), &visible);
    }


    void Layer::render_primitive_id()              { Log::add("To be implemented"); }
    void Layer::render_constant_color(int layerid) { Log::add("To be implemented"); }
    void Layer::destroy() {}

    // TODO remove
    void LayerManager::render() {
        for (auto& [_, layer] : *this)
            if (groups[layer->group].visible && layer->visible)
                layer->render();
    }

    void LayerManager::render_group(int group_id) {
        for (int l = 0; l < God::layers.size(); ++l) {
            Layer &layer = God::layers[l];
            if (layer.group == group_id && groups[group_id].visible && layer.visible)
                layer.render();
        }
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
//      ObjectId(path().layers[layer_name]).broadcast(events::KILLED);
    }

    void LayerManager::kill(std::string layer_name){
        int idx = find(layer_name);        
        kill(idx);
    }

    std::optional<Group> LayerManager::get_group(const std::string& name) {
        for (auto &group : groups) {
            if (group.name == name)
                return group;
        }
        return std::nullopt;
    }

    int LayerManager::add_group(const std::string& name) {
        groups.emplace_back(max_group++, Group::BlendOp::NONE, name);
        return max_group - 1;
    }

    void LayerManager::remove_group(int group_id) {
        auto it = std::find_if(groups.begin(), groups.end(), [&group_id](const auto &g) { return g.id == group_id; });
        if (it != groups.end())
            groups.erase(it);
    }

    bool LayerManager::is_group_empty(int group_id) {
        for (int l = 0; l < size(); ++l) {
            auto &layer = Registry<Layer>::operator[](l);
            if (layer.group == group_id)
                return false;
        }

        return true;
    }

    std::vector<std::reference_wrapper<Layer>> LayerManager::from_group(int group_id) {
        std::vector<std::reference_wrapper<Layer>> result;
        for (auto& e : items) {
            if (e.object->group == group_id) {
                result.push_back(*e.object);
            }
        }
        return result;
    }



    void LayerManager::destroy() {
        for (auto& [name, obj] : *this)
            obj->destroy();
    }
