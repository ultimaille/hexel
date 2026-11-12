#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>


struct PolyLineLayer : public Layer {
    AttributePath attr_id;
    enum RenderType {
        PLAIN,
        EDGE, EDGE_REPEAT,
        POINT, POINT_SPHERE
    };

    RenderType rt = PLAIN;

    int primitive_type();

    //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name
    inline bool attr_active()           { return (ContainerBase *)attr_id != nullptr; }// deso pour le jeu de mot
 
    SegmentRenderer edge_renderer;
    PointRenderer point_renderer;

    PolyLineLayer();
    void generate_gui(std::string name);
    void set_default_values() override;
    void reset();
    void init(MeshPath obj);
    void render();
    void destroy();
    void bind_attribute(AttributePath attr_id);


    template<class T> bool update_typed();
    void update();

    void prepare_data_without_attribute(PolyLine& pl, EdgeAttribute<bool>& visible_pl, PointAttribute<float>& value_pl, EdgeAttribute<int>& primitive_id);
    template<class T> void update_data_for_scalar_edge_attribute();
    template<class T> void update_data_for_scalar_edge_attribute_as_repeat();
    template<class T> void update_data_for_scalar_point_attribute();
    template<class T> void update_data_for_scalar_point_attribute_as_sphere();
    void update_data_for_plain();
};
