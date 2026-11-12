#include "layer_ui.h"
#include <layers/basic_layers.h>
#include <layers/basic_renderers.h>

bool RendererUI::ambiant_slider() {
    return ImGui::SliderFloat("##slider_ambiant", &renderer().ambient_prop, 0.0f, 1.0f, "%.3f", 0);
}

bool RendererUI::color_picker() {
    return ImGui::ColorEdit3("##picker_color", (float*)&renderer().color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
}

bool RendererUI::colormap_combo() {
    int prev = renderer().texture_id;
    bool used = ColormapCombo("##combo_colormap", renderer().texture_id);
    if (renderer().texture_id != prev) {
        glDeleteTextures(1, &renderer().colormap);
        load_colormap(renderer().texture_id, renderer().colormap);
    }
    return used;
}

void RendererUI::data_range_input() {
    ImGui::InputFloat2("##input_data_range", renderer().value_range, "%.2f", ImGuiInputTextFlags_ReadOnly);
}

void RendererUI::show_range_input() {
    ImGui::InputFloat2("##input_show_range", renderer().texture_range);
}

void RendererUI::auto_range_checkbox() {
    if (ImGui::Checkbox("##autorange", &renderer().auto_range)){
        renderer().texture_range[0] = renderer().value_range[0];
        renderer().texture_range[1] = renderer().value_range[1];
    }
}

void RendererUI::texture_repeat_input() {
    if (ImGui::InputInt("##texture_repeat", &renderer().texture_repeat))
        renderer().texture_repeat = std::clamp(renderer().texture_repeat, 0, 1000000);
}

bool PointRendererUI::radius_input(PointRenderer &r) {
    return ImGui::InputInt("##point_size", &r.radius_in_pixel);
}

void PointRendererUI::show_parameters(RendererUIMode mode) {
    if (mode == RendererUIMode::BASIC) {
        PropertyTable("point_table_basic_parameter", {
            {"Style", [&] {
                color_picker();
                ImGui::PushItemWidth(80);
                radius_input(r);
                ImGui::PopItemWidth();
            }}
        });
    }
    if (mode == RendererUIMode::ADVANCED) {
        PropertyTable("point_table_basic_parameter", {
            {"Ambient", [&] {
                ambiant_slider();
            }},
        });
    }
}

void TriangleRendererUI::show_parameters(RendererUIMode mode) {
    if (mode == RendererUIMode::BASIC) {
        PropertyTable("tri_table_basic_parameter", {
            {"Style", [&] {
                color_picker();
            }}
        });
    }
    if (mode == RendererUIMode::ADVANCED) {
        PropertyTable("tri_table_basic_parameter", {
            {"Ambient", [&] {
                ambiant_slider();
            }},
        });
    }
}

void SegmentRendererUI::show_parameters(RendererUIMode mode) {
    if (mode == RendererUIMode::BASIC) {
        PropertyTable("tri_table_basic_parameter", {
            {"Style", [&]() {
                color_picker();
                ImGui::SameLine();
                ImGui::PushItemWidth(80);
                ImGui::InputInt("##input_width", &r.line_width);
                r.line_width = std::clamp(r.line_width, int(r.width_range[0]), int(r.width_range[1]));
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::SliderFloat("##slider_scale", &r.origin_scale, 0.4f, 1.0f, "%.3f", 0);
            }}
        });
    }
    if (mode == RendererUIMode::ADVANCED) {
        PropertyTable("tri_table_basic_parameter", {
            {"Ambient", [&]() {
                ambiant_slider();
            }},
        });
    }
}

template<class T>
ObjectId LayerUI::attribute_combo(std::string mm_name, std::string mesh_name, ObjectId attr_id, T attr) {
    auto new_attr_id = AttributeCombo(mm_name, mesh_name, attr, attr_id);
    return new_attr_id;
}

int LayerUI::render_type_combo(std::vector<std::tuple<std::string, int>> items, int cur_render_type) {
    int has_selected = -1;
    int current_item = cur_render_type;

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
                has_selected = rt;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    return has_selected;
}

void PointSetLayerUI::show_parameters(LayerUIMode mode) {

    PointRendererUI renderer_ui(l.point_renderer);
    renderer_ui.show_parameters(static_cast<RendererUI::RendererUIMode>(mode));

    if (mode == LayerUIMode::BASIC) {
        PropertyTable("point_set_layer_table_basic_parameter", {
            {"Mask value", [&] {
                if (ImGui::Checkbox("##cb_mask_value", &l.mask)) l.update();
                ImGui::SameLine();
                if (l.mask) if (ImGui::InputInt("##input_mask_value", &l.skip_value)) { l.update(); }
            }},
        });
    }

    if (mode == LayerUIMode::ATTRIBUTE) {

        std::map<std::string, std::vector<std::tuple<std::string, int>>> render_types_by_attr_type {
            {"points", {{"ATTRIBUTE_SPHERE", PointSetLayer::RenderType::ATTRIBUTE_SPHERE}}
        }};

        PropertyTable("pointset_layer_table_attribute", {
            {"Attribute", [&] {
                auto new_attr_id = attribute_combo(l.mesh.names[0], l.mesh.names.size() > 1 ? l.mesh.names[1] : "", l.attr_id, static_cast<PointSetAttributes>(l.mesh));
                if (new_attr_id.path != ObjectType::NA) l.bind_attribute(new_attr_id);
            }},
            {"Render type", [&] {
                if (l.rt != PointSetLayer::RenderType::PLAIN) {
                    int last_rt = static_cast<int>(l.rt);
                    int new_rt = render_type_combo(render_types_by_attr_type[l.attr_primitive()], static_cast<int>(l.rt));
                    if (new_rt != -1) {
                        l.rt = static_cast<PointSetLayer::RenderType>(new_rt);
                        if (l.rt != last_rt)
                            l.reset();
                    }
                }
            }},
            {"Colormap", [&] {
                renderer_ui.colormap_combo();
            }},
            {"Data range", [&] { renderer_ui.data_range_input(); }},
            {"Show range", [&] { 
                renderer_ui.show_range_input(); 
                ImGui::SameLine();
                renderer_ui.auto_range_checkbox();
            }},
            {"Texture repeat", [&] { renderer_ui.texture_repeat_input(); }}
        });
    }
}

void PolyLineLayerUI::show_parameters(LayerUIMode mode) {

    std::unique_ptr<RendererUI> renderer_ui;
    if (l.rt == PolyLineLayer::RenderType::POINT)
        renderer_ui = std::make_unique<PointRendererUI>(l.point_renderer);
    else 
        renderer_ui = std::make_unique<SegmentRendererUI>(l.edge_renderer);

    renderer_ui->show_parameters(static_cast<RendererUI::RendererUIMode>(mode));


    if (mode == LayerUIMode::ATTRIBUTE) {

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

        PropertyTable("polyline_layer_table_attribute", {
            {"Attribute", [&] {
                auto new_attr_id = attribute_combo(l.mesh.names[0], l.mesh.names.size() > 1 ? l.mesh.names[1] : "", l.attr_id, static_cast<PolyLineAttributes>(l.mesh));
                if (new_attr_id.path != ObjectType::NA) l.bind_attribute(new_attr_id);
            }},
            {"Render type", [&] {
                if (l.rt != PolyLineLayer::RenderType::PLAIN) {
                    int last_rt = static_cast<int>(l.rt);
                    int new_rt = render_type_combo(render_types_by_attr_type[l.attr_primitive()], static_cast<int>(l.rt));
                    if (new_rt != -1) {
                        l.rt = static_cast<PolyLineLayer::RenderType>(new_rt);
                        if (l.rt != last_rt)
                            l.reset();
                    }
                }
            }},
            {"Colormap", [&] {
                renderer_ui->colormap_combo();
            }},
            {"Data range", [&] { renderer_ui->data_range_input(); }},
            {"Show range", [&] { 
                renderer_ui->show_range_input(); 
                ImGui::SameLine();
                renderer_ui->auto_range_checkbox();
            }},
            {"Texture repeat", [&] { renderer_ui->texture_repeat_input(); }}
        });
    }
}

void TrianglesLayerUI::show_parameters(LayerUIMode mode) {

    std::unique_ptr<RendererUI> renderer_ui;
    if (l.rt == TrianglesLayer::RenderType::POINT_SPHERE)
        renderer_ui = std::make_unique<PointRendererUI>(l.point_renderer);
    else 
        renderer_ui = std::make_unique<TriangleRendererUI>(l.tri_renderer);

    renderer_ui->show_parameters(static_cast<RendererUI::RendererUIMode>(mode));

    if (mode == LayerUIMode::BASIC) {
        switch (l.rt) {
            case TrianglesLayer::RenderType::CORNER_GLYPH: {
                PropertyTable("point_set_layer_table_basic_parameter", {
                    {"Shrink", [&] {
                        if (ImGui::SliderFloat("##slider_shrink", &l.shrink, 0.0f, .9f, "%.2f", 0)) l.reset();
                    }},
                    {"Corner size", [&] {
                        if (ImGui::SliderFloat("##slider_corner_size", &l.corner_size, 0.1f, 1.f, "%.2f", 0)) l.reset();
                    }},
                });
                break;
            }
        }
    }
    if (mode == LayerUIMode::ATTRIBUTE) {

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

        PropertyTable("triangle_layer_table_attribute", {
            {"Attribute", [&] {
                auto new_attr_id = attribute_combo(l.mesh.names[0], l.mesh.names.size() > 1 ? l.mesh.names[1] : "", l.attr_id, static_cast<SurfaceAttributes>(l.mesh));
                if (new_attr_id.path != ObjectType::NA) l.bind_attribute(new_attr_id);
            }},
            {"Render type", [&] {
                if (l.rt != TrianglesLayer::RenderType::PLAIN) {
                    int last_rt = l.rt;
                    int new_rt = render_type_combo(render_types_by_attr_type[l.attr_primitive()], static_cast<int>(l.rt));
                    if (new_rt != -1) {
                        l.rt = static_cast<TrianglesLayer::RenderType>(new_rt);
                        if (l.rt != last_rt)
                            l.reset();
                    }
                }
            }},
            {"Colormap", [&] {
                renderer_ui->colormap_combo();
            }},
            {"Data range", [&] { renderer_ui->data_range_input(); }},
            {"Show range", [&] { 
                renderer_ui->show_range_input(); 
                ImGui::SameLine();
                renderer_ui->auto_range_checkbox();
            }},
            {"Texture repeat", [&] { renderer_ui->texture_repeat_input(); }}
        });
    }
}

void HexahedraLayerUI::show_parameters(LayerUIMode mode) {
    std::unique_ptr<RendererUI> renderer_ui;
    if (l.rt == HexahedraLayer::RenderType::POINT_SPHERE)
        renderer_ui = std::make_unique<PointRendererUI>(l.point_renderer);
    else 
        renderer_ui = std::make_unique<TriangleRendererUI>(l.tri_renderer);

    renderer_ui->show_parameters(static_cast<RendererUI::RendererUIMode>(mode));

    if (mode == LayerUIMode::BASIC) {
        PropertyTable("hexahedra_layer_table_basic_parameter", {
            {"Shrink", [&] {
                float prev_shrink = l.shrink;
                ImGui::SliderFloat("##slider_shrink", &l.shrink, 0.0f, .9f, "%.2f", 0);
                if (prev_shrink != l.shrink) l.reset();
            }},
        });
    }
    if (mode == LayerUIMode::ATTRIBUTE) {

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

        PropertyTable("hexahedra_layer_table_attribute", {
            {"Attribute", [&] {
                auto new_attr_id = attribute_combo(l.mesh.names[0], l.mesh.names.size() > 1 ? l.mesh.names[1] : "", l.attr_id, static_cast<VolumeAttributes>(l.mesh));
                if (new_attr_id.path != ObjectType::NA) l.bind_attribute(new_attr_id);
            }},
            {"Render type", [&] {
                if (l.rt != HexahedraLayer::RenderType::PLAIN) {
                    int last_rt = static_cast<int>(l.rt);
                    int new_rt = render_type_combo(render_types_by_attr_type[l.attr_primitive()], static_cast<int>(l.rt));
                    if (new_rt != -1) {
                        l.rt = static_cast<HexahedraLayer::RenderType>(new_rt);
                        if (l.rt != last_rt)
                            l.reset();
                    }
                }
            }},
            {"Colormap", [&] {
                renderer_ui->colormap_combo();
            }},
            {"Data range", [&] { renderer_ui->data_range_input(); }},
            {"Show range", [&] { 
                renderer_ui->show_range_input(); 
                ImGui::SameLine();
                renderer_ui->auto_range_checkbox();
                ImGui::SameLine();
                
            }},
            {"Texture repeat", [&] { renderer_ui->texture_repeat_input(); }}
        });
    }
}

