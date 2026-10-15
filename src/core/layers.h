#pragma once
#include <optional>
#include <core/event.h>

struct Layer {
    Layer();
    virtual ~Layer() = default;
    virtual void reset();
    virtual void generate_gui(std::string name);

    virtual void render() = 0;
    virtual void render_primitive_id();
    virtual void render_constant_color(int layerid);
    virtual void destroy();

    virtual void bind_attribute(ObjectId attr_id){ Log::error("This layer does not support attribute binding."); }
    virtual int primitive_id(int vertex_id);

    bool visible = true;
    ObjectId mesh;
    virtual int primitive_type() { Log::error("This layer has no identified primtives"); return -1; } // number in the collections PointAttributes, PolyLineAttributes, SurfaceAttributes and VolumeAttributes

    static inline int max_id = 0;
    int id;
    int group;
};

struct LayerManager: private Registry<Layer> {
    int size()                                 { return Registry<Layer>::size(); }
    Layer& operator[](int i)             { return Registry<Layer>::operator[](i); }
    Layer& operator[](std::string s)     { return Registry<Layer>::operator[](s); }
    bool contains(std::string s)               { return Registry<Layer>::contains(s); }
    int find(std::string str)                  { return Registry<Layer>::find(str); }
    std::string ith_name(int i)                { return items[i].name; }
    void swap(int i, int j)                    { std::swap(items[i], items[j]); }

    template<class T> T& add(std::string str, int group)  { 
        auto &layer = emplace_back<T>(str); 
        layer.group = group;
        return layer; 
    }

    int add_group() {
        groups.push_back(max_group++);
        return max_group - 1;
    }

    void render();
    void handle(Event event);
    std::optional<std::reference_wrapper<Layer>> find_by_id(int id);
    void kill(std::string layer_name);

    void destroy();


    std::vector<int> groups;
    static inline int max_group = 0;

};


