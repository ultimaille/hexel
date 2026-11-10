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
    if (type == "points") return { ImVec4(0.20f, 0.45f, 0.90f, 1.0f),
                                    ImVec4(0.30f, 0.55f, 1.00f, 1.0f),
                                    ImVec4(0.15f, 0.35f, 0.75f, 1.0f) }; // blue

    if (type == "corners") return { ImVec4(0.95f, 0.55f, 0.15f, 1.0f),
                                    ImVec4(1.00f, 0.65f, 0.25f, 1.0f),
                                    ImVec4(0.80f, 0.45f, 0.10f, 1.0f) }; // orange

    if (type == "facets")  return { ImVec4(0.20f, 0.70f, 0.20f, 1.0f),
                                    ImVec4(0.30f, 0.80f, 0.30f, 1.0f),
                                    ImVec4(0.15f, 0.60f, 0.15f, 1.0f) }; // green
    return {};
}

static bool ColoredButton(const char* label, std::string type) {
    ButtonPalette p = GetPalette(type);
    ImGui::PushStyleColor(ImGuiCol_Button,        p.base);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, p.hovered);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  p.active);
    bool clicked = ImGui::Button(label, ImVec2(-FLT_MIN, 0.0f));
    ImGui::PopStyleColor(3);
    return clicked;
}

static void AttributeRange(float value_range[2], float texture_range[2], int &texture_repeat, bool &auto_range) {

    ImGui::InputFloat2("Data range##input_data_range", value_range, "%.2f", ImGuiInputTextFlags_ReadOnly);
    ImGui::InputFloat2("Show range##input_show_range", texture_range);

    ImGui::SameLine();
    if (ImGui::Checkbox("autorange##autorange", &auto_range)){
        texture_range[0] = value_range[0];
        texture_range[1] = value_range[1];
    }

    if (ImGui::InputInt("texture repeat##texture_repeat", &texture_repeat))
        texture_repeat = std::clamp(texture_repeat, 0, 1000000);

}

struct AttrItem {
    ObjectId id;
    std::string key;    // unique: section + name
    std::string label;  // display text
    ImVec4 color;
};

static ObjectId AttributeCombo(std::string mm_name, std::string mesh_name, SurfaceAttributes &attr, ObjectId& current_id) {
    ObjectId sel_attr_id;
    static ImGuiTextFilter filter;
    // static std::string current_key;

    // Collect all attributes into a flat list
    std::vector<AttrItem> items;
    auto collect = [&](auto &containers, const char *section) {
        for (auto &container : containers) {
            ObjectId id{ObjectType::TRIANGLES_ATTR,
                        {mm_name, mesh_name, attribute_type(&*container.ptr), section, container.name}};
            items.push_back({
                id,
                std::string(section) + "/" + container.name,
                std::string(section) + ": " + container.name,
                GetPalette(section).base   // same color key ColoredButton uses
            });
        }
    };
    collect(attr.points,  "points");
    collect(attr.corners, "corners");
    collect(attr.facets,  "facets");

    // Find the currently displayed item, if any
    const AttrItem *current = nullptr;
    for (const auto &it : items)
        if (it.id == current_id) { current = &it; break; }


    // Preview text takes the color of the current selection
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

            // Color swatch
            ImGui::ColorButton("##swatch", it.color,
                               ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoBorder,
                               ImVec2(12, 12));
            ImGui::SameLine();

            // Colored item label
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

#include <layers/basic_layers.h>
struct RendererUI {

    RendererUI(Renderer &r) : r(r) {}

    void show_basic_parameters();

    bool ambiant_slider();

    bool color_picker();

    bool color_texture_slider();

    bool colormap_combo();

    void attribute_range();

    protected:

    bool radius_input(PointRenderer &r);

    void show_basic_parameters(PointRenderer &r);

    void show_basic_parameters(TriangleRenderer &r);

    private:
    Renderer &r;
};


struct LayerUI {

    LayerUI(Layer &l) : l(l), renderer_ui(l.renderer()) {}

    void attribute_combo(ObjectId attr_id);
    bool render_type_combo(std::vector<std::tuple<std::string, int>> items);

    void show();
    void show_basic_parameters();
    void show_basic_parameters(TrianglesLayer &l);
    void show_basic_parameters(HexahedraLayer &l);
    void show_attribute_parameters();
    void show_attribute_parameters(TrianglesLayer &l);
    void show_attribute_parameters(HexahedraLayer &l);

    private:
    Layer &l;
    RendererUI renderer_ui;
};