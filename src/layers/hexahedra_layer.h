#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>


struct HexahedraLayer : public Layer {
    ObjectId attr_id;
    bool render_as_spheres=false;
    float shrink = .1;

    inline bool attr_active()           { return attr_id.names.size() == 5; }// deso pour le jeu de mot
    inline std::string attr_name()      { um_assert(attr_active()); return attr_id.names[4]; }
    inline std::string attr_primitive() { um_assert(attr_active()); return attr_id.names[3]; }
    inline std::string attr_type()      { um_assert(attr_active()); return attr_id.names[2]; }
    inline bool use_point_renderer()    { return !attr_id.names.empty() && attr_primitive() == "points" && render_as_spheres; }

    TriangleRenderer tri_renderer;
    PointRenderer point_renderer;

    HexahedraLayer();
    void generate_gui(std::string name);
    void reset();
    void init(ObjectId obj);
    void render();
    virtual int primitive_id(int vertex_id);
    void destroy();
    void bind_attribute(ObjectId attr_id);

    void prepare_data_without_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    );
    template<class T> void prepare_data_for_scalar_cell_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    );
    template<class T> void prepare_data_for_scalar_facet_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    );
    template<class T> void prepare_data_for_scalar_corner_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    );
    template<class T> void prepare_data_for_scalar_point_attribute(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name,
        Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri
    );

    template<class T> void update_data_for_scalar_point_attribute_as_sphere(
        Hexahedra& hex, CellAttribute<bool>& visible_hex, VolumeAttributes& attr, std::string attr_name
        );
};








