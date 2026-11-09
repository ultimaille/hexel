#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct TrianglesBuilder {
    // TrianglesDecomposer(TriangleRenderer &tri_renderer) : tri_renderer(tri_renderer) {}
    // virtual void update(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value) {}
    virtual void transform(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) {};

    protected:
    // TriangleRenderer& tri_renderer;
};

struct ScalarFacetTrianglesBuilder : public TrianglesBuilder {
    // using TrianglesDecomposer::TrianglesDecomposer;

    // void update(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value) override {

    //     FacetAttribute<int> primitive_id(m, 0);
    //     CornerAttribute<float> value_out(m, 0);
        
    //     for (auto &f : m.iter_facets()) primitive_id[f] = f;
    //     for (auto &h : m.iter_halfedges()) value_out[h] =  value[h / 3];

    //     tri_renderer.update(m, visible, value_out, primitive_id);
    // }

    void transform(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) override {
        FacetAttribute<int> primitive_id(m, 0);
        CornerAttribute<float> value_out(m, 0);
        
        for (auto &f : m.iter_facets()) primitive_id[f] = f;
        for (auto &h : m.iter_halfedges()) value_out[h] =  value[h / 3];
        f(m, visible, value_out, primitive_id);
    };

};

struct ScalarCornerAttrBuilder : public TrianglesBuilder {
    // using TrianglesDecomposer::TrianglesDecomposer;

    // void update(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value) override {

    //     FacetAttribute<int> primitive_id(m, -1);
    //     CornerAttribute<float> value_out(m, 0);

    //     for (auto &f : m.iter_facets()) primitive_id[f] = f;
    //     for (auto &h : m.iter_halfedges()) value_out[h] = static_cast<float>(value[h]);

    //     tri_renderer.update(m, visible, value_out, primitive_id);
    // }

};

struct ScalarCornerGlyphAttrBuilder : public TrianglesBuilder {
    // using TrianglesDecomposer::TrianglesDecomposer;

    // void update(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value) {}


    void transform(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) override {

        Triangles tri_glyph;
        tri_glyph.points.create_points(m.ncorners() * 3);
        tri_glyph.create_facets(m.ncorners());

        FacetAttribute<bool> visible_glyph(tri_glyph);
        CornerAttribute<float> value_glyph(tri_glyph);
        FacetAttribute<int> primitive_id(tri_glyph);

        for (auto &f : m.iter_facets()) {
            Triangle3 t = f;
            vec3 bary = t.bary_verts();
            for (int lv = 0; lv < 3; ++lv) {
                vec3 from = f.vertex(lv);
                vec3 to = f.vertex((lv + 1) % 3);
                vec3 opp = f.vertex((lv + 2) % 3);
                auto h = f.halfedge(lv);

                vec3 a = from * (1. - shrink) + bary * shrink;
                vec3 b = to * (1. - shrink) + bary * shrink;
                vec3 c = opp * (1. - shrink) + bary * shrink;

                vec3 d0 = (c - a);
                vec3 d1 = (c - b);

                // opp
                float size = corner_size * .5;

                tri_glyph.points[h * 3] = a * size + c * (1. - size);
                tri_glyph.points[h * 3 + 1] = b * size + c * (1. - size);
                tri_glyph.points[h * 3 + 2] = c;

                for (int i = 0; i < 3; ++i) {
                    tri_glyph.vert(h, i) = h * 3 + i;

                    value_glyph[h * 3 + i] = static_cast<float>(value[h]);
                    visible_glyph[h] = visible[f];
                    primitive_id[h] = h;
                }

            }
        }

        f(tri_glyph, visible_glyph, value_glyph, primitive_id);
    }

    float shrink = 0.f;
    float corner_size = 1.f;

};

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

    void build_and_update_tri_renderer(TrianglesBuilder &builder);


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

    std::map<RenderType, TrianglesBuilder> rendering_type {
        {FACET, ScalarFacetTrianglesBuilder()},
        {CORNER, ScalarCornerAttrBuilder()},
        {CORNER_GLYPH, ScalarCornerGlyphAttrBuilder()},
    };

    float shrink = 0.f;
    float corner_size = 1.f;
};
