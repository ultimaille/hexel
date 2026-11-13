#pragma once
#include "core/event.h"

// Horrible global variables
extern Path grad_and_drop_objectid;


inline std::string attribute_type(ContainerBase* ptr){
    if (dynamic_cast<AttributeContainer<bool> *>(ptr) != nullptr) return "bool";
    if (dynamic_cast<AttributeContainer<char> *>(ptr) != nullptr) return "char";
    if (dynamic_cast<AttributeContainer<int> *>(ptr) != nullptr) return "int";
    if (dynamic_cast<AttributeContainer<float> *>(ptr) != nullptr) return "float";
    if (dynamic_cast<AttributeContainer<double> *>(ptr) != nullptr) return "double";
    if (dynamic_cast<AttributeContainer<vec2> *>(ptr) != nullptr) return "vec2";
    if (dynamic_cast<AttributeContainer<vec3> *>(ptr) != nullptr) return "vec3";
    return "UNKNOWN";
}

inline std::string to_string(Path obj) {
    std::string res;
    for (int i = 0; i < obj.path.size(); i++)
        if (!obj.path[i].empty())
            res += (i == 0 ? ": " : "->") + obj.path[i];
    return  res;
}

inline void show_object_id(Path obj){
    Log::add(to_string(obj));
}

bool FilePopup(const char* id, std::string in, std::string& out, std::vector<const char*> extensions);
void ImGui_help(const char* desc);
using namespace events;
void look_at_pointset(PointSet& ps);
void load_mm_with_default_layers(std::string path);

