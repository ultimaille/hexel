#pragma once
#include <imgui.h>
#include <ultimaille/all.h>
#include <core/objectid.h>
#include <core/event.h>
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

void AttributeRange(float value_range[2], float texture_range[2], int &texture_repeat, bool &auto_range) {

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

// // Create UI bloc for attribute management
// static void AttributeBloc(Layer &layer) {
//     auto &r = layer.current_renderer();
//     int prev = r.texture_id;
//     ColormapCombo("Colormap", r.texture_id);
//     if (r.texture_id != prev) {
//         glDeleteTextures(1, &r.colormap);
//         load_colormap(r.texture_id, r.colormap);
//     }

//     auto new_attr_id = AttributeCombo(layer.mesh.names[0], layer.mesh.names[1], layer.mesh, layer.attr_id);
//     if (new_attr_id.path != ObjectType::NA) layer.bind_attribute(new_attr_id);

//     AttributeRange(r.value_range, r.texture_range, r.texture_repeat, r.auto_range);
// }

struct LayerUI {

    static void generate(Renderer &r) {
        ImGui::PushItemWidth(120);
        ImGui::SliderFloat("ambient_M##slider", &r.ambient_prop, 0.0f, 1.0f, "%.3f", 0);
        
        ImGui::ColorEdit3("ConstColor##color", (float*)&r.color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
        ImGui::SameLine();
        ImGui::SliderFloat("color/texture##slider", &r.color_map_prop, 0.0f, 1.0f, "%.3f", 0);

        ImGui::PopItemWidth();
    }

    static void generate(PointRenderer& r) {
        generate(static_cast<Renderer&>(r));

        ImGui::PushItemWidth(80);
        ImGui::InputInt("point size##point_size", &r.radius_in_pixel);
        ImGui::PopItemWidth();
    }

};