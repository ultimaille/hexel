#pragma once
#include "layers/basic_layers.h"
#include "core/event.h"

// Horrible global variables
extern ObjectId grad_and_drop_objectid;


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

inline std::string to_string(events::ObjectType path) {
    switch (path) {
    case events::ObjectType::KEYBOARD:          return "keyboard";
    case events::ObjectType::MOUSE:             return "mouse";
    case events::ObjectType::CAMERA:            return "camera";
    case events::ObjectType::PANEL:             return "panel";
    case events::ObjectType::LAYER:             return "layer";
    case events::ObjectType::MULTIMESH:         return "multimesh";
    case events::ObjectType::POINTSET:          return "pointset";
    case events::ObjectType::POLYLINES:         return "polylines";
    case events::ObjectType::TRIANGLES:         return "triangles";
    case events::ObjectType::QUADS:             return "quads";
    case events::ObjectType::POLYGONS:          return "polygons";
    case events::ObjectType::TETRAHEDRA:        return "tetrahedra";
    case events::ObjectType::HEXAHEDRA:         return "hexahedra";
    case events::ObjectType::WEDGES:            return "wedges";
    case events::ObjectType::PYRAMIDS:          return "pyramids";

    case events::ObjectType::POINTSET_ATTR:     return "pointset";
    case events::ObjectType::POLYLINES_ATTR:    return "polylines";
    case events::ObjectType::TRIANGLES_ATTR:    return "triangles";
    case events::ObjectType::QUADS_ATTR:        return "quads";
    case events::ObjectType::POLYGONS_ATTR:     return "polygons";
    case events::ObjectType::TETRAHEDRA_ATTR:   return "tetrahedra";
    case events::ObjectType::HEXAHEDRA_ATTR:    return "hexahedra";
    case events::ObjectType::WEDGES_ATTR:       return "wedges";
    case events::ObjectType::PYRAMIDS_ATTR:     return "pyramids";
    };
    return "XXX";
}
inline std::string to_string(ObjectId obj) {
    std::string res = to_string(obj.path);
    for (int i = 0; i < obj.names.size(); i++)
        if (!obj.names[i].empty())
            res += (i == 0 ? ": " : "->") + obj.names[i];
    return  res;
}

inline void show_object_id(ObjectId obj){
    Log::add(to_string(obj));
}

bool FilePopup(const char* id, std::string in, std::string& out, std::vector<const char*> extensions);
using namespace events;
void look_at_pointset(PointSet& ps);
void load_mm_with_default_layers(std::string path);
