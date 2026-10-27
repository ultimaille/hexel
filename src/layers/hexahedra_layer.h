#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

// // That pass from mesh / attribute data to readable data for renderers
// struct Preparator {

//     Preparator(Layer &layer) : layer(layer) {}

//     virtual void prepare_data(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id) = 0;

//     protected:
//     Layer& layer;
// };

// struct HexahedraLayer;

// struct PreparatorWithoutData : Preparator {



//     void prepare_data(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id) override {
//         Hexahedra& hex = layer.mesh;
//         VolumeAttributes& attr = layer.mesh;
//         CellAttribute<bool> visible_hex("visible", attr, hex, true);
//         tri.points.create_points(hex.ncorners());
//         tri.create_facets(2 * hex.nfacets());
//         for (auto c : hex.iter_corners()) tri.points[c] = c.vertex();
//         for (auto c : hex.iter_cells()){
//             vec3 G = Hexahedron(c).bary_verts();
//             for (auto v : c.iter_corners())
//                 tri.points[v] = (1. - layer.shrink) * tri.points[v] + shrink * G;
//         }
//         int map[2][3] = { {0,1,2},{2,3,0} };
//         for (auto f : hex.iter_facets()) 
//             FOR(t, 2)FOR(v, 3)tri.vert(2 * f + t,v) = f.corner(map[t][v]);
//         for (auto f : hex.iter_facets()) FOR(i, 2) visible_tri[2 * f + i] = visible_hex[f.cell()];
//         for (auto f : hex.iter_facets()) FOR(t, 2) primitive_id[2 * f + t] = f;
//     }
// };

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








