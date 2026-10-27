#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct HexahedraLayer : public Layer {
    ObjectId attr_id;
    enum RenderType {
        PLAIN,
        CELL,
        FACET,
        CORNER, CORNER_GLYPH,
        POINT, POINT_SPHERE
    };

    RenderType rt = PLAIN; // Note: set default value here

    int primitive_type();
    float shrink = .1;

    //attr_id.names = 0->mm_name, 1->hex_name, 2->attribute_type, 3->attribute_primitive, 4->attribute_name
    inline bool attr_active()           { return attr_id.names.size() == 5; }// deso pour le jeu de mot
    inline std::string attr_name()      { um_assert(attr_active()); return attr_id.names[4]; }
    inline std::string attr_primitive() { um_assert(attr_active()); return attr_id.names[3]; }
    inline std::string attr_type()      { um_assert(attr_active()); return attr_id.names[2]; }

    TriangleRenderer tri_renderer;
    PointRenderer point_renderer;

    HexahedraLayer();
    void generate_gui(std::string name);
    void reset();
    void init(ObjectId obj);
    void render();
    void destroy();
    void bind_attribute(ObjectId attr_id);


    template<class T>
    void prepare_and_update();

    void prepare_data_without_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    );
    template<class T> void prepare_data_for_scalar_cell_attribute(
       Triangles& tri, CornerAttribute<float>& value_tri
    );
    template<class T> void prepare_data_for_scalar_facet_attribute(
        Triangles& tri, CornerAttribute<float>& value_tri
    );
    template<class T> void prepare_data_for_scalar_corner_attribute(
        Triangles& tri, CornerAttribute<float>& value_tri
    );
    template<class T> void prepare_data_for_scalar_corner_glyph_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    );
    template<class T> void prepare_data_for_scalar_point_attribute(
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id
    );
    template<class T> void update_data_for_scalar_point_attribute_as_sphere(PointAttribute<bool>& visible_pts, PointAttribute<float>& value_float, PointAttribute<int>& primitive_id);

};








