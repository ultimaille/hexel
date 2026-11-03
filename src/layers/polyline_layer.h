#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>


struct PolyLineLayer : public Layer {
    ObjectId attr_id;
    enum RenderType {
        PLAIN,
        EDGE, EDGE_REPEAT,
        POINT, POINT_SPHERE
    };

    RenderType rt = PLAIN;

    int primitive_type();

    //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name
    inline bool attr_active()           { return attr_id.names.size() == 5; }// deso pour le jeu de mot
    inline std::string attr_name()      { um_assert(attr_active()); return attr_id.names[4]; }
    inline std::string attr_primitive() { um_assert(attr_active()); return attr_id.names[3]; }
    inline std::string attr_type()      { um_assert(attr_active()); return attr_id.names[2]; }

    SegmentRenderer edge_renderer;
    PointRenderer point_renderer;

    PolyLineLayer();
    void generate_gui(std::string name);
    void reset();
    void init(ObjectId obj);
    void render();
    void destroy();
    void bind_attribute(ObjectId attr_id);


    template<class T>void update_typed();
    void update();

    void prepare_data_without_attribute(PolyLine& pl, EdgeAttribute<bool>& visible_pl, PointAttribute<float>& value_pl, EdgeAttribute<int>& primitive_id);
    template<class T> void update_data_for_scalar_edge_attribute();
    template<class T> void update_data_for_scalar_edge_attribute_as_repeat();
    template<class T> void update_data_for_scalar_point_attribute();
    template<class T> void update_data_for_scalar_point_attribute_as_sphere();
    void update_data_for_plain();
};
