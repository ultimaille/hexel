#pragma once 
#include "basic_renderers.h"
#include <core/layers.h>

struct TrianglesBuilder {

    virtual void transform(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) {};

};

struct TrianglesBuilder2 {

    virtual void transform(Triangles &m, PointAttribute<bool> &visible, PointAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) {};

};

struct PlainTrianglesBuilder : public TrianglesBuilder {

    void transform(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) override {
        FacetAttribute<int> primitive_id(m, 0);
        CornerAttribute<float> value_out(m, 0);
        for (auto &f : m.iter_facets()) primitive_id[f] = f;
        f(m, visible, value_out, primitive_id);
    };

};

struct ScalarFacetTrianglesBuilder : public TrianglesBuilder {

    void transform(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) override {
        FacetAttribute<int> primitive_id(m, 0);
        CornerAttribute<float> value_out(m, 0);
        
        for (auto &f : m.iter_facets()) primitive_id[f] = f;
        for (auto &h : m.iter_halfedges()) value_out[h] =  value[h / 3];
        f(m, visible, value_out, primitive_id);
    };

};

struct ScalarPointAttrTrianglesBuilder : public TrianglesBuilder2 {
    void transform(Triangles &m, PointAttribute<bool> &visible, PointAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) override {
        FacetAttribute<bool> visible_out(m, false);
        FacetAttribute<int> primitive_id(m, 0);
        CornerAttribute<float> value_out(m, 0);
        for (auto &f : m.iter_facets()) primitive_id[f] = f;
        // Note: need the mesh to be connected...
        // for (auto &h : tri.iter_halfedges()) value_tri[h] = value[h.from()];
        // Note: below no need the mesh to be connected
        for (auto &f : m.iter_facets()) {
            for (int lv = 0; lv < 3; ++lv) {
                auto h = f.halfedge(lv);
                auto v = f.vertex(lv);
                value_out[h] = value[v];
                // at least one point visible => facet visible
                if (visible[v]) visible_out[f] = true;
            }
        }
        f(m, visible_out, value_out, primitive_id);
    }
};





struct ScalarCornerAttrBuilder : public TrianglesBuilder {

    void transform(Triangles &m, FacetAttribute<bool> &visible, FacetAttribute<float> &value, std::function<void(Triangles&, FacetAttribute<bool>&, CornerAttribute<float>&, FacetAttribute<int>&)> f) override {
        FacetAttribute<int> primitive_id(m, 0);
        CornerAttribute<float> value_out(m, 0);

        for (auto &f : m.iter_facets()) primitive_id[f] = f;
        for (auto &h : m.iter_halfedges()) value_out[h] = value[h];

        f(m, visible, value_out, primitive_id);
    }
};

struct ScalarCornerGlyphAttrBuilder : public TrianglesBuilder {

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
    inline bool attr_active()           { return (ContainerBase *)attr_id != nullptr; }// deso pour le jeu de mot
                                                                                       //
    TrianglesLayer();
    void reset() override;
    void init(MeshPath obj);
    void render();
    void bind_attribute(AttributePath attr_id);
    void destroy();
    void facet_value_to_float(Triangles &m, FacetAttribute<float>& value_out);
    void point_value_to_float(Triangles &m, PointAttribute<float>& value_out);

    template<class T> bool update_typed();
    void update();


    AttributeBase::TYPE primitive_type();


    void prepare_data_without_attribute(Triangles& tri, FacetAttribute<bool>& visible_tri, CornerAttribute<float>& value_tri, FacetAttribute<int>& primitive_id);
    template<class T> void update_data_for_scalar_facet_attribute();
    template<class T> void update_data_for_scalar_corner_attribute();
    template<class T> void update_data_for_scalar_corner_glyph_attribute();
    template<class T> void update_data_for_scalar_point_attribute();
    template<class T> void update_data_for_scalar_point_attribute_as_sphere();
    void update_data_for_plain();

    void build_and_update_tri_renderer(TrianglesBuilder &builder);
    void build_and_update_tri_renderer(TrianglesBuilder2 &builder);

    AttributePath attr_id;
    enum RenderType {
        PLAIN,
        FACET,
        CORNER, CORNER_GLYPH, HALFEDGE_GLYPH,
        POINT, POINT_SPHERE
    };

    RenderType rt = PLAIN;

    TriangleRenderer tri_renderer;
    PointRenderer point_renderer;

    float shrink = 0.f;
    float corner_size = 1.f;

    std::map<RenderType, std::unique_ptr<TrianglesBuilder>> rendering_type;
    std::map<RenderType, std::unique_ptr<TrianglesBuilder2>> rendering_type2;

};
