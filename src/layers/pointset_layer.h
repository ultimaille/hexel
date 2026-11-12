#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct PointSetLayer : public Layer{

    PointSetLayer();

    int primitive_type();
    void generate_gui(std::string name);
    void set_default_values() override;
    void reset() override;
    void init(MeshPath obj);
    void render();
    void bind_attribute(AttributePath attr_id);
    void destroy();


    template<class T> bool update_typed();
    void update();

    // void prepare_data_without_attribute(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id);
    template<class T> void update_data_for_scalar_point_attribute_as_sphere();
    void update_data_for_plain();

    // TODO put this in layer or utils function as its copy / paste from hexahedra layer...
    //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name
    inline bool attr_active()           { return (ContainerBase *)attr_id != nullptr; }// deso pour le jeu de mot
                                                                                       //
    AttributePath attr_id;
    enum RenderType {
        PLAIN,
        ATTRIBUTE_SPHERE
    };
    bool mask = false;
    int skip_value = -1;

    RenderType rt = PLAIN;

    PointRenderer point_renderer;
};
