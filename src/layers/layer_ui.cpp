#include "layer_ui.h"
#include <layers/basic_layers.h>
#include <layers/basic_renderers.h>

void RendererUI::show_basic_parameters() {
    // Note: for now just dispatch using dynamic cast but visitor 
    // can be used instead for cleaner version
    if (PointRenderer *pr = dynamic_cast<PointRenderer*>(&r)) {
        show_basic_parameters(*pr);
    } else if (TriangleRenderer *tr = dynamic_cast<TriangleRenderer*>(&r)) {
        show_basic_parameters(*tr);
    } else if (SegmentRenderer *sr = dynamic_cast<SegmentRenderer*>(&r)) {
        show_basic_parameters(*sr);
    } else {
        Log::error("Renderer isn't supported by RendererUI");
    }
}

bool RendererUI::ambiant_slider() {
    return ImGui::SliderFloat("ambient##slider_ambiant", &r.ambient_prop, 0.0f, 1.0f, "%.3f", 0);
}

bool RendererUI::color_picker() {
    return ImGui::ColorEdit3("ConstColor##color", (float*)&r.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
}

bool RendererUI::color_texture_slider() {
    return ImGui::SliderFloat("color/texture##slider", &r.color_map_prop, 0.0f, 1.0f, "%.3f", 0);
}

bool RendererUI::colormap_combo() {
    int prev = r.texture_id;
    bool used = ColormapCombo("Colormap", r.texture_id);
    if (r.texture_id != prev) {
        glDeleteTextures(1, &r.colormap);
        load_colormap(r.texture_id, r.colormap);
    }
    return used;
}

void RendererUI::attribute_range() {
    AttributeRange(r.value_range, r.texture_range, r.texture_repeat, r.auto_range);
}

bool RendererUI::radius_input(PointRenderer &r) {
    return ImGui::InputInt("point size##point_size", &r.radius_in_pixel);
}

void RendererUI::show_basic_parameters(PointRenderer &r) {
    ImGui::PushItemWidth(80);
    radius_input(r);
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(120);
    color_picker();
    ImGui::SameLine();
    color_texture_slider();
    ambiant_slider();
    ImGui::PopItemWidth();
}

void RendererUI::show_basic_parameters(TriangleRenderer &r) {
    ImGui::PushItemWidth(120);
    color_picker();
    ImGui::SameLine();
    color_texture_slider();
    ambiant_slider();
    ImGui::PopItemWidth();
}

void RendererUI::show_basic_parameters(SegmentRenderer &r) {
    ImGui::PushItemWidth(80);
    ImGui::InputInt("line width", &r.line_width);
    r.line_width = std::clamp(r.line_width, int(r.width_range[0]), int(r.width_range[1]));
    ImGui::SliderFloat("origin scale##slider", &r.origin_scale, 0.4f, 1.0f, "%.3f", 0);
    ImGui::PopItemWidth();
}

template<class T>
void LayerUI::attribute_combo(ObjectId attr_id) {
    std::string mm_name = l.mesh.names[0];
    std::string mesh_name = l.mesh.names.size() > 1 ? l.mesh.names[1] : "";
    auto new_attr_id = AttributeCombo(mm_name, mesh_name, static_cast<T&>(l.mesh), attr_id);
    if (new_attr_id.path != ObjectType::NA) l.bind_attribute(new_attr_id);
}

bool LayerUI::render_type_combo(std::vector<std::tuple<std::string, int>> items) {
    bool has_selected = false;
    int current_item = l.render_type();

    std::string label = "";
    for (auto &[l, i] : items) {
        if (current_item == i) {
            label = l;
            break;
        }
    }

    if (ImGui::BeginCombo("RenderType", label.c_str())) {
        for (int n = 0; n < (int)items.size(); n++) {
            auto [name, rt] = items[n];
            bool is_selected = (current_item == rt);

            if (ImGui::Selectable(name.c_str(), is_selected)) {
                l.render_type(rt);
                has_selected = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return has_selected;
}

void LayerUI::show() {
    show_basic_parameters();
    show_attribute_parameters();
}

void LayerUI::show_basic_parameters() {

    renderer_ui.show_basic_parameters();

    if (PointSetLayer *pl = dynamic_cast<PointSetLayer*>(&l)) {
        show_basic_parameters(*pl);
    } else if (PolyLineLayer *pl = dynamic_cast<PolyLineLayer*>(&l)) {
        show_basic_parameters(*pl);
    } else if (TrianglesLayer *tl = dynamic_cast<TrianglesLayer*>(&l)) {
        show_basic_parameters(*tl);
    } else if (HexahedraLayer *hl = dynamic_cast<HexahedraLayer*>(&l)) {
        show_basic_parameters(*hl);
    } else {
        Log::error("Layer isn't supported by LayerUI");
    }
}

void LayerUI::show_basic_parameters(PointSetLayer &l) {
    if (ImGui::Checkbox("mask a value##autorange", &l.mask)) l.update();
    if (l.mask) if (ImGui::InputInt("mask value", &l.skip_value)){ l.update(); }
}

void LayerUI::show_basic_parameters(PolyLineLayer &l) {

}

void LayerUI::show_basic_parameters(TrianglesLayer &l) {
    switch (static_cast<TrianglesLayer::RenderType>(l.render_type())) {
        case TrianglesLayer::RenderType::CORNER_GLYPH: {
            float tmp_shrink = l.shrink;
            float tmp_corner_size = l.corner_size;
            ImGui::SliderFloat("shrink", &l.shrink, 0.0f, .9f, "%.2f", 0);
            ImGui::SliderFloat("corner_size", &l.corner_size, 0.1f, 1.f, "%.2f", 0);
            if (tmp_shrink != l.shrink || tmp_corner_size != l.corner_size) l.reset();
            break;
        }
    }
}

void LayerUI::show_basic_parameters(HexahedraLayer &l) {
    float prev_shrink = l.shrink;
    ImGui::SliderFloat("shrink", &l.shrink, 0.0f, .9f, "%.2f", 0);
    if (prev_shrink != l.shrink) l.reset();
}

void LayerUI::show_attribute_parameters() {
    if (PointSetLayer *pl = dynamic_cast<PointSetLayer*>(&l)) {
        show_attribute_parameters(*pl);
    } else if (PolyLineLayer *pl = dynamic_cast<PolyLineLayer*>(&l)) {
        show_attribute_parameters(*pl);
    } else if (TrianglesLayer *tl = dynamic_cast<TrianglesLayer*>(&l)) {
        show_attribute_parameters(*tl);
    } else if (HexahedraLayer *hl = dynamic_cast<HexahedraLayer*>(&l)) {
        show_attribute_parameters(*hl);
    } else {
        Log::error("Layer isn't supported by LayerUI");
    }
}

void LayerUI::show_attribute_parameters(PointSetLayer &l) {
    if (ImGui::CollapsingHeader("Attributes")) {

        renderer_ui.colormap_combo();
        attribute_combo<PointSetAttributes>(l.attr_id);
        renderer_ui.attribute_range();

        std::map<std::string, std::vector<std::tuple<std::string, int>>> render_types_by_attr_type {
            {"points", {{"ATTRIBUTE_SPHERE", PointSetLayer::RenderType::ATTRIBUTE_SPHERE}}
        }};

        if (l.attr_active()) {
            int last_rt = static_cast<int>(l.rt);
            if (render_type_combo(render_types_by_attr_type[l.attr_primitive()])) {
                if (l.rt != last_rt)
                    l.reset();
            }
        }
    }
}

void LayerUI::show_attribute_parameters(PolyLineLayer &l) {
    if (ImGui::CollapsingHeader("Attributes")) {

        renderer_ui.colormap_combo();
        attribute_combo<PolyLineAttributes>(l.attr_id);
        renderer_ui.attribute_range();

        std::map<std::string, std::vector<std::tuple<std::string, int>>> render_types_by_attr_type {
            {"points", {
                {"POINT", PolyLineLayer::RenderType::POINT}, 
                {"POINT_SPHERE", PolyLineLayer::RenderType::POINT_SPHERE}}
            },
            {"edges", {
                {"EDGE", PolyLineLayer::RenderType::EDGE}, 
                {"EDGE_REPEAT", PolyLineLayer::RenderType::EDGE_REPEAT}}
            }
        };

        if (l.attr_active()) {
            int last_rt = static_cast<int>(l.rt);
            if (render_type_combo(render_types_by_attr_type[l.attr_primitive()])) {
                if (l.rt != last_rt)
                    l.reset();
            }
        }
    }
}

void LayerUI::show_attribute_parameters(TrianglesLayer &l) {
    if (ImGui::CollapsingHeader("Attributes")) {

        renderer_ui.colormap_combo();
        attribute_combo<SurfaceAttributes>(l.attr_id);
        renderer_ui.attribute_range();

        std::map<std::string, std::vector<std::tuple<std::string, int>>> render_types_by_attr_type {
            {"points", {
                {"POINT", TrianglesLayer::RenderType::POINT}, 
                {"POINT_SPHERE", TrianglesLayer::RenderType::POINT_SPHERE}}
            },
            {"corners", {
                {"CORNER", TrianglesLayer::RenderType::CORNER}, 
                {"CORNER_GLYPH", TrianglesLayer::RenderType::CORNER_GLYPH}, 
                {"HALFEDGE_GLYPH", TrianglesLayer::RenderType::HALFEDGE_GLYPH}}
            },
            {"facets", {
                {"FACET", TrianglesLayer::RenderType::FACET}}
            },
        };

        if (l.attr_active()) {
            int last_rt = static_cast<int>(l.rt);
            if (render_type_combo(render_types_by_attr_type[l.attr_primitive()])) {
                if (l.rt != last_rt)
                    l.reset();
            }
        }
    }
}

void LayerUI::show_attribute_parameters(HexahedraLayer &l) {
    if (ImGui::CollapsingHeader("Attributes")) {

        renderer_ui.colormap_combo();
        attribute_combo<VolumeAttributes>(l.attr_id);
        renderer_ui.attribute_range();

        std::map<std::string, std::vector<std::tuple<std::string, int>>> render_types_by_attr_type {
            {"points", {
                {"POINT", HexahedraLayer::RenderType::POINT}, 
                {"POINT_SPHERE", HexahedraLayer::RenderType::POINT_SPHERE}}
            },
            {"corners", {
                {"CORNER", HexahedraLayer::RenderType::CORNER}, 
                {"CORNER_GLYPH", HexahedraLayer::RenderType::CORNER_GLYPH}}
            },
            {"facets", {{"FACET", HexahedraLayer::RenderType::FACET}}},
            {"cells", {{"CELL", HexahedraLayer::RenderType::CELL}}}
        };

        if (l.attr_active()) {
            int last_rt = static_cast<int>(l.rt);
            if (render_type_combo(render_types_by_attr_type[l.attr_primitive()])) {
                if (l.rt != last_rt)
                    l.reset();
            }
        }
    }
}

