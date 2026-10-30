#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct PointSetLayer : public Layer{

    PointSetLayer();

    int primitive_type();
    void generate_gui(std::string name);
    void reset() override;
    void init(ObjectId obj);
    void render();
    void bind_attribute(ObjectId attr_id);
    void destroy();


    template<class T>void update_typed();
    void update();

    // void prepare_data_without_attribute(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id);
    template<class T> void update_data_for_scalar_point_attribute_as_sphere();
    template<class T> void update_data_for_scalar_point_attribute_as_sized_sphere();
    void update_data_for_plain();

    // TODO put this in layer or utils function as its copy / paste from hexahedra layer...
    //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name
    inline bool attr_active()           { return attr_id.names.size() == 4; }// deso pour le jeu de mot
    inline std::string attr_name()      { um_assert(attr_active()); return attr_id.names[3]; }
    inline std::string attr_primitive() { um_assert(attr_active()); return attr_id.names[2]; }
    inline std::string attr_type()      { um_assert(attr_active()); return attr_id.names[1]; }

    ObjectId attr_id;
    enum RenderType {
        PLAIN,
        POINT_SPHERE
    };

    RenderType rt = PLAIN;

    PointRenderer point_renderer;
};
