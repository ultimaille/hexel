#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct TrianglesLayer : public Layer{


    // TODO put this in layer or utils function as its copy / paste from hexahedra layer...
    //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name
    inline bool attr_active()           { return attr_id.names.size() == 5; }// deso pour le jeu de mot
    inline std::string attr_name()      { um_assert(attr_active()); return attr_id.names[4]; }
    inline std::string attr_primitive() { um_assert(attr_active()); return attr_id.names[3]; }
    inline std::string attr_type()      { um_assert(attr_active()); return attr_id.names[2]; }

    TrianglesLayer();
    void generate_gui(std::string name);
    void reset() override;
    void init(ObjectId obj);
    void render();
    void bind_attribute(ObjectId attr_id);
    void destroy();

    template<class T>void update_typed();
    void update();

    void prepare_data_without_attribute(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id);
    template<class T> void update_data_for_scalar_facet_attribute();
    template<class T> void update_data_for_scalar_corner_attribute();
    template<class T> void update_data_for_scalar_corner_glyph_attribute();
    template<class T> void update_data_for_scalar_halfedge_glyph_attribute();
    template<class T> void update_data_for_scalar_point_attribute();
    template<class T> void update_data_for_scalar_point_attribute_as_sphere();
    void update_data_for_plain();

    ObjectId attr_id;
    enum RenderType {
        PLAIN,
        FACET,
        CORNER, CORNER_GLYPH, HALFEDGE_GLYPH,
        POINT, POINT_SPHERE
    };

    RenderType rt = PLAIN;

    TriangleRenderer tri_renderer;
    PointRenderer point_renderer;
};
