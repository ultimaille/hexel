#pragma once
#include <imgui.h>
#include <ultimaille/all.h>

#include <core/objectid.h>
#include <core/event.h>
#include <core/layers.h>
#include <misc/purgatory.h>

using namespace UM;

struct ButtonPalette { ImVec4 base, hovered, active; };

static ButtonPalette GetPalette(std::string type)
{
    if (type == "points") 
        return { ImVec4(0.20f, 0.45f, 0.90f, 1.0f),
            ImVec4(0.30f, 0.55f, 1.00f, 1.0f),
            ImVec4(0.15f, 0.35f, 0.75f, 1.0f) }; // blue

    if (type == "corners") 
        return { ImVec4(0.95f, 0.55f, 0.15f, 1.0f),
            ImVec4(1.00f, 0.65f, 0.25f, 1.0f),
            ImVec4(0.80f, 0.45f, 0.10f, 1.0f) }; // orange

    if (type == "edges")   
        return { ImVec4(1.00f, 1.00f, 0.00f, 1.0f),
            ImVec4(1.00f, 1.00f, 0.35f, 1.0f),
            ImVec4(0.80f, 0.80f, 0.00f, 1.0f) }; // yellow

    if (type == "facets")  
        return { ImVec4(0.20f, 0.70f, 0.20f, 1.0f),
            ImVec4(0.30f, 0.80f, 0.30f, 1.0f),
            ImVec4(0.15f, 0.60f, 0.15f, 1.0f) }; // green

    if (type == "cells")   
        return { ImVec4(1.00f, 1.00f, 1.00f, 1.0f),
            ImVec4(0.92f, 0.92f, 0.92f, 1.0f),
            ImVec4(0.80f, 0.80f, 0.80f, 1.0f) }; // white
    return {};
}

// static bool ColoredButton(const char* label, std::string type) {
//     ButtonPalette p = GetPalette(type);
//     ImGui::PushStyleColor(ImGuiCol_Button,        p.base);
//     ImGui::PushStyleColor(ImGuiCol_ButtonHovered, p.hovered);
//     ImGui::PushStyleColor(ImGuiCol_ButtonActive,  p.active);
//     bool clicked = ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f));
//     ImGui::PopStyleColor(3);
//     return clicked;
// }

struct AttrItem {
    ObjectId id;
    std::string key;    // unique: section + name
    std::string label;  // display text
    ImVec4 color;
};

template <typename C>
static ObjectId MeshAttrId(const std::string &mm_name, const std::string &mesh_name,
                           C &container, const char *section) {
    ObjectId id{ObjectType::TRIANGLES_ATTR,
                {mm_name, mesh_name, attribute_type(&*container.ptr), section, container.name}};
    return id;
}

template <typename Containers, typename MakeId>
static void AddItems(std::vector<AttrItem> &items, Containers &containers,
                     const char *section, MakeId make_id) {
    for (auto &container : containers) {
        items.push_back({
            make_id(container, section),
            std::string(section) + "/" + container.name,
            std::string(section) + ": " + container.name,
            GetPalette(section).base   // same color key ColoredButton uses
        });
    }
}

static ObjectId DrawAttributeCombo(const std::vector<AttrItem> &items, ObjectId &current_id) {
    static ImGuiTextFilter filter;
    ObjectId sel_attr_id;

    const AttrItem *current = nullptr;
    for (const auto &it : items)
        if (it.id == current_id) { current = &it; break; }

    const bool has_current = current != nullptr;
    if (has_current)
        ImGui::PushStyleColor(ImGuiCol_Text, current->color);
    const bool open = ImGui::BeginCombo("##attr_combo",
                                        has_current ? current->label.c_str() : "Select attribute...");
    if (has_current)
        ImGui::PopStyleColor();

    if (open) {
        filter.Draw("##attr_filter");
        ImGui::Separator();

        for (const auto &it : items) {
            if (!filter.PassFilter(it.label.c_str()))
                continue;

            ImGui::PushID(it.key.c_str());

            ImGui::ColorButton("##swatch", it.color,
                               ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
                               ImVec2(12, 12));
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, it.color);
            const bool clicked = ImGui::Selectable(it.label.c_str(), it.id == current_id);
            ImGui::PopStyleColor();

            ImGui::PopID();

            if (clicked) {
                current_id = it.id;
                sel_attr_id = it.id;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndCombo();
    }

    return sel_attr_id;
}

static ObjectId AttributeCombo(std::string mm_name, std::string mesh_name,
                               SurfaceAttributes &attr, ObjectId &current_id) {
    auto make = [&](auto &c, const char *sec) { return MeshAttrId(mm_name, mesh_name, c, sec); };
    std::vector<AttrItem> items;
    AddItems(items, attr.points,  "points",  make);
    AddItems(items, attr.corners, "corners", make);
    AddItems(items, attr.facets,  "facets",  make);
    return DrawAttributeCombo(items, current_id);
}

static ObjectId AttributeCombo(std::string mm_name, std::string mesh_name,
                               VolumeAttributes &attr, ObjectId &current_id) {
    auto make = [&](auto &c, const char *sec) { return MeshAttrId(mm_name, mesh_name, c, sec); };
    std::vector<AttrItem> items;
    AddItems(items, attr.points,       "points",  make);
    AddItems(items, attr.cell_corners, "corners", make);
    AddItems(items, attr.cell_facets,  "facets",  make);
    AddItems(items, attr.cells,        "cells",   make);
    return DrawAttributeCombo(items, current_id);
}

static ObjectId AttributeCombo(std::string mm_name, std::string mesh_name,
                               PolyLineAttributes &attr, ObjectId &current_id) {
    auto make = [&](auto &c, const char *sec) { return MeshAttrId(mm_name, mesh_name, c, sec); };
    std::vector<AttrItem> items;
    AddItems(items, attr.points, "points", make);
    AddItems(items, attr.edges,  "edges",  make);
    return DrawAttributeCombo(items, current_id);
}

static ObjectId AttributeCombo(std::string mm_name, std::string mesh_name,
                               PointSetAttributes &attr, ObjectId &current_id) {
    auto make = [&](auto &c, const char *sec) {
        ObjectId id{ObjectType::POINTSET_ATTR,
                    {mm_name, attribute_type(&*c.ptr), sec, c.name}};
        return id;
    };
    std::vector<AttrItem> items;
    AddItems(items, attr.points, "points", make);
    return DrawAttributeCombo(items, current_id);
}

static void PropertyTable(std::string name, std::vector<std::pair<std::string, std::function<void()>>> rows) {
    if (ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
        for (auto &[row_name, row_f] : rows) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(row_name.c_str());
            ImGui::TableSetColumnIndex(1);
            row_f();
        }
        ImGui::EndTable();
    }
}

#include <layers/basic_layers.h>
struct RendererUI {

    enum RendererUIMode { BASIC, ADVANCED };

    virtual Renderer& renderer() = 0;
    virtual void show_parameters(RendererUIMode mode) = 0;

    bool ambiant_slider();
    bool color_picker();
    bool colormap_combo();
    void data_range_input();
    void show_range_input();
    void auto_range_checkbox();
    void texture_repeat_input();
};

struct PointRendererUI : RendererUI {
    PointRendererUI(PointRenderer &r) : r(r) {}

    bool radius_input(PointRenderer &r);
    
    void show_parameters(RendererUIMode mode) override;
    Renderer& renderer() override { return static_cast<Renderer&>(r); }

    PointRenderer &r;
};

struct SegmentRendererUI : RendererUI {
    SegmentRendererUI(SegmentRenderer &r) : r(r) {}

    void show_parameters(RendererUIMode mode) override;
    Renderer& renderer() override { return static_cast<Renderer&>(r); }

    SegmentRenderer &r;
};

struct TriangleRendererUI : RendererUI {
    TriangleRendererUI(TriangleRenderer &r) : r(r) {}

    void show_parameters(RendererUIMode mode) override;
    Renderer& renderer() override { return static_cast<Renderer&>(r); }

    TriangleRenderer &r;
};

struct LayerUI {

    enum LayerUIMode { BASIC, ADVANCED, ATTRIBUTE };

    template<class T>
    ObjectId attribute_combo(std::string mm_name, std::string mesh_name, ObjectId attr_id, T attr);
    int render_type_combo(std::vector<std::tuple<std::string, int>> items, int cur_render_type);

    virtual void show_parameters(LayerUIMode mode) = 0;
};

struct PointSetLayerUI : public LayerUI {
    PointSetLayerUI(PointSetLayer &l) : l(l) {}

    void show_parameters(LayerUIMode mode) override;

    private:
    PointSetLayer &l;
};

struct PolyLineLayerUI : public LayerUI {
    PolyLineLayerUI(PolyLineLayer &l) : l(l) {}

    void show_parameters(LayerUIMode mode) override;

    private:
    PolyLineLayer &l;
};

struct TrianglesLayerUI : public LayerUI {
    TrianglesLayerUI(TrianglesLayer &l) : l(l) {}

    void show_parameters(LayerUIMode mode) override;

    private:
    TrianglesLayer &l;
};

struct HexahedraLayerUI : public LayerUI {
    HexahedraLayerUI(HexahedraLayer &l) : l(l) {}

    void show_parameters(LayerUIMode mode) override;

    private:
    HexahedraLayer &l;
};