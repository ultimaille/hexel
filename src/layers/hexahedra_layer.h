#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>


struct HexahedraLayer : public Layer {
    AttributePath attr_id;
    enum RenderType {
        PLAIN,
        CELL,
        FACET,
        CORNER, CORNER_GLYPH,
        POINT, POINT_SPHERE
    };

    RenderType rt = PLAIN;

    int primitive_type();
    float shrink = .1;

    //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name
    inline bool attr_active()           { return (ContainerBase *)attr_id != nullptr; }// deso pour le jeu de mot

    TriangleRenderer tri_renderer;
    PointRenderer point_renderer;

    HexahedraLayer();
    void generate_gui(std::string name);
    void reset();
    void init(MeshPath obj);
    void render();
    void destroy();
    void bind_attribute(AttributePath attr_id);


    template<class T> bool update_typed();
    void update();

    void prepare_data_without_attribute(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id);
    template<class T> void update_data_for_scalar_cell_attribute();
    template<class T> void update_data_for_scalar_facet_attribute();
    template<class T> void update_data_for_scalar_corner_attribute();
    template<class T> void update_data_for_scalar_corner_glyph_attribute();
    template<class T> void update_data_for_scalar_point_attribute();
    template<class T> void update_data_for_scalar_point_attribute_as_sphere();
    void update_data_for_plain();
};








