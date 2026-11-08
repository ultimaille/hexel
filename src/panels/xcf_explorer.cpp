#include "xcf_explorer.h"
#include <core/all.h>
#include "panels/property_explorer.h"

#include <layers/pointset_layer.h> 
#include <layers/triangle_layer.h> 
#include <layers/polyline_layer.h> 
#include <layers/hexahedra_layer.h> 
#include <layers/ssao.h>

void XCFExplorer::object_id_drag_source(ObjectId id, std::string name) {
    if (ImGui::BeginDragDropSource()) {
        grad_and_drop_objectid = id;
        ObjectId* ptr = &grad_and_drop_objectid;
        ImGui::SetDragDropPayload("OBJECTID", &ptr, sizeof(ObjectId*));
        ImGui::Text("Moving %s", name.c_str());
        ImGui::EndDragDropSource();
    }
};

struct AttributeInfoPayload {
    int primitive;
    char name[256];
    char type[50];
};

void XCFExplorer::attribute_info_drag_source(AttributeInfo attr_info, std::string name) {
    if (ImGui::BeginDragDropSource()) {
        AttributeInfoPayload payload;
        std::snprintf(payload.name, sizeof(payload.name), "%s", attr_info.name.c_str());
        std::snprintf(payload.type, sizeof(payload.type), "%s", attr_info.type.c_str());
        payload.primitive = static_cast<int>(attr_info.primitive);
        ImGui::SetDragDropPayload("ATTR_INFO", &payload, sizeof(AttributeInfoPayload));
        ImGui::Text("Moving %s", name.c_str());
        ImGui::EndDragDropSource();
    }
};

bool XCFExplorer::open_mesh(ObjectIdSet& selected, ObjectId mesh_id, std::string gna){
    auto mm_name = mesh_id.names[0];
    auto mesh_name = mesh_id.names.size() > 1 ? mesh_id.names[1] : "";
    //ImGui::Separator();
    bool open_mesh;
    {
        NodeColor ncol(1);
        open_mesh = ImGui::TreeNodeEx(label(gna + "." + mesh_name, mm_name), flag(selected.contains(mesh_id), false));
    }

    if (ImGui::IsItemClicked()) switch_selection(selected, mesh_id);
    object_id_drag_source(mesh_id, mm_name + "." + mesh_name);
    return open_mesh;
};

void XCFExplorer::add_attribute(ObjectIdSet& selected, ObjectId mesh_id, AttributeInfo attr_info, std::string name){
    bool open_attr;
    {
        NodeColor ncol(3);
        open_attr = ImGui::TreeNodeEx(label(name, mesh_id.names[0]), flag(selected.contains(mesh_id), true));
        // Note: ici on sait qu'on veux un attribut ! on sait tjs quand on veux un attribut / mesh / ... pas besoin d'avoir un object id generique
        attribute_info_drag_source(attr_info, name);
    }
    if (open_attr){
        // if (ImGui::IsItemClicked()) switch_selection(selected, attr_id);
        ImGui::TreePop();
    }
};

void XCFExplorer::generate_gui() {
    static ObjectIdSet selected([&](const ObjectId& a, const ObjectId& b) { if (a.path != b.path) return a.path < b.path; return a.names < b.names; });

    {// sync with property panel
        PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(ObjectId(root().panels["property_window"])));
        std::vector<ObjectId> to_kill;
        for (auto sel : selected) if (sel.names.size() == 1 && sel.path == LAYER) to_kill.push_back(sel);
        for (auto id : to_kill)selected.erase(id);
        for (auto id : pan.layers)
            if (!selected.contains(id))
                selected.insert(id);
    }
    ImGui::Begin("XCFViewer", nullptr);

    //=====================================================================================
    //         CREATE MM
    //=====================================================================================

    if (ImGui::Button(label("New MultiMesh"))) ImGui::OpenPopup("Pick MultiMesh Name##NewMultiMeshPopup");

    ImGui::SetNextWindowPos(ImGui::GetCursorPos());

    if (ImGui::BeginPopupModal("Pick MultiMesh Name##NewMultiMeshPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
        static char tmp[1024] = "toto";
        ImGui::InputText(label("", "NewMultiMeshPopup"), tmp, 1024);
        ImGui::SameLine();
        if (ImGui::SmallButton("OK")) {
            ImGui::CloseCurrentPopup();
            God::xcf.add(tmp);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }






    //=====================================================================================
    //         START THE TREE
    //=====================================================================================
    std::vector<std::string> mm2kill;

    // ==> MultiMesh
    for (auto& [mm_name, mm] : God::xcf) {

        ImGui::Separator();
        static bool closable_mm_group = true;
        ObjectId mm_id(MULTIMESH, mm_name);
        bool open_mm;
        {
            NodeColor ncol(0);
            open_mm = (ImGui::TreeNodeEx(label(mm_name, "mm"), flag(selected.contains(mm_id))));
        }
        object_id_drag_source(mm_id, mm_name);
        if (ImGui::IsItemClicked()) switch_selection(selected, mm_id);



        if (ImGui::BeginPopupContextItem(label("folder_context", mm_name))){
            if (ImGui::Button("Delete"))
                mm2kill.push_back(mm_name);

            if (ImGui::BeginMenu("create")){
                static char tmp[1024] = "toto";
                ImGui::Text("name"); ImGui::SameLine();
                ImGui::InputText(label("", "gloglo"), tmp, 1024);

                if (ImGui::MenuItem(label("Polyline", mm_name))){
                    God::xcf.add_mesh(root().xcf[mm_name].polylines[tmp]);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem(label("Triangles", mm_name))){
                    God::xcf.add_mesh(root().xcf[mm_name].triangles[tmp]);
                    ImGui::CloseCurrentPopup();
                }
                // TODO: uncomment when layers implemented !
                // if (ImGui::MenuItem(label("Quads", mm_name))){
                    // God::xcf.add_mesh(ObjectId(ObjectType::QUADS, { mm_name,tmp }));
                    // ImGui::CloseCurrentPopup();
                // }
                // if (ImGui::MenuItem(label("Tetrahedra", mm_name))){
                    // God::xcf.add_mesh(ObjectId(ObjectType::TETRAHEDRA, { mm_name,tmp }));
                    // ImGui::CloseCurrentPopup();
                // }
                if (ImGui::MenuItem(label("Hexahedra", mm_name))){
                    God::xcf.add_mesh(root().xcf[mm_name].hexahedra[tmp]);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }


        if (open_mm){
            // ==> pointset
            ObjectId mesh_id(POINTSET, mm_name);

            bool open_pointset = open_mesh(selected, {POINTSET, { mm_name }}, "pointset");

            if (ImGui::BeginPopupContextItem(label("pointset_context", mm_name + "pointset"))) {

                if (ImGui::BeginMenu("create")) {
                    
                    static char tmp[1024] = "toto";
                    ImGui::Text("name"); ImGui::SameLine();
                    ImGui::InputText(label("", "gloglo"), tmp, 1024);
                    
                    if (ImGui::MenuItem(label("PointSetLayer", mm_name))) {
                        auto group_opt = God::layers.find_group(mm_name);
                        int group_id = group_opt ? group_opt.value().id : God::layers.add_group(mm_name);
                        auto &layer = God::layers.add<PointSetLayer>(tmp, group_id);
                        layer.init(mesh_id);
                        layer.set_default_values();
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            if (ImGui::IsItemClicked()) switch_selection(selected, mesh_id);
            if (open_pointset) {
                for (auto& it : mm.pointset_attributes.points) { 
                    add_attribute(
                        selected, 
                        mesh_id, 
                        { .name = it.name, .primitive = AttributeBase::TYPE::POINTS, .type = attribute_type(&*it.ptr)},
                        "points." + it.name
                    );
                }
                add_mesh_layers(selected, {POINTSET, { mm_name }}, "pointset");
                ImGui::TreePop();
            }






            // ==> polylines
            std::vector<std::string> polyline2kill;
            for (auto& [mesh_name, obj] : mm.polylines) {
                ObjectId mesh_id{POLYLINES, { mm_name, mesh_name }};

                bool open = open_mesh(selected, mesh_id, "polylines");

                if (ImGui::BeginPopupContextItem(label("killpolylinepopup", mm_name + mesh_name))) {
                    if (ImGui::Button("Delete")){
                        polyline2kill.push_back(mesh_name);
                        ImGui::CloseCurrentPopup();
                    }

                    if (ImGui::BeginMenu("create")) {
                        
                        static char tmp[1024] = "toto";
                        ImGui::Text("name"); ImGui::SameLine();
                        ImGui::InputText(label("", "gloglo"), tmp, 1024);
                        
                        if (ImGui::MenuItem(label("PolyLineLayer", mm_name))) {
                            auto group_opt = God::layers.find_group(mm_name);
                            int group_id = group_opt ? group_opt.value().id : God::layers.add_group(mm_name);
                            auto &layer = God::layers.add<PolyLineLayer>(tmp, group_id);
                            layer.init(mesh_id);
                            layer.set_default_values();
                            ImGui::CloseCurrentPopup();
                        }


                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
                if (!open) continue;

                for (auto& it : obj.attributes.points) 
                    add_attribute(
                        selected, mesh_id, 
                        { .name = it.name, .primitive = AttributeBase::TYPE::POINTS, .type = attribute_type(&*it.ptr)},
                        "points." + it.name
                    );
                for (auto& it : obj.attributes.edges) 
                    add_attribute(
                        selected, mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::EDGES, .type = attribute_type(&*it.ptr)},
                        "edges." + it.name
                    );

                add_mesh_layers(selected, {POLYLINES, { mm_name, mesh_name }}, "polylines");
                ImGui::TreePop();
            }

            // TODO: can refactor duplicate (triangle / quads; tetrahedra / hexahedra), resolving some ObjectType at compile-time
            // But wait for new object id before doing this !
            // ==> triangles
            std::vector<std::string> tri2kill;
            for (auto& [mesh_name, obj] : mm.triangles) {

                ObjectId mesh_id{TRIANGLES, { mm_name, mesh_name }};

                bool open = open_mesh(selected, mesh_id, "triangles");
                if (ImGui::BeginPopupContextItem(label("killtripopup", mm_name + mesh_name))) {
                    if (ImGui::Button("Delete")) {
                        tri2kill.push_back(mesh_name);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::BeginMenu("create")) {
                        
                        static char tmp[1024] = "toto";
                        ImGui::Text("name"); ImGui::SameLine();
                        ImGui::InputText(label("", "gloglo"), tmp, 1024);
                        
                        if (ImGui::MenuItem(label("TrianglesLayer", mm_name))) {
                            auto group_opt = God::layers.find_group(mm_name);
                            int group_id = group_opt ? group_opt.value().id : God::layers.add_group(mm_name);
                            auto &layer = God::layers.add<TrianglesLayer>(tmp, group_id);
                            layer.init(mesh_id);
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }

                if (!open) continue;
                for (auto& it : obj.attributes.points) 
                    add_attribute(
                        selected, 
                        mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::POINTS, .type = attribute_type(&*it.ptr)},
                        "points." + it.name
                    );
                for (auto& it : obj.attributes.corners) 
                    add_attribute(
                        selected, 
                        mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::CORNERS, .type = attribute_type(&*it.ptr)},
                        "corners." + it.name
                    );
                for (auto& it : obj.attributes.facets) 
                    add_attribute(
                        selected, 
                        mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::FACETS, .type = attribute_type(&*it.ptr)},
                        "facets." + it.name
                    );
                add_mesh_layers(selected, {TRIANGLES, { mm_name, mesh_name }}, "triangles");
                ImGui::TreePop();

            }

            // ==> hexahedra
            std::vector<std::string> hex2kill;
            for (auto& [mesh_name, obj] : mm.hexahedra) {
                ObjectId mesh_id = {HEXAHEDRA, { mm_name, mesh_name }};
                bool open = open_mesh(selected, mesh_id, "hexahedra");
                if (ImGui::BeginPopupContextItem(label("killhexpopup", mm_name + mesh_name))){
                    if (ImGui::Button("Delete")){
                        hex2kill.push_back(mesh_name);
                        ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::BeginMenu("create")) {
                        
                        static char tmp[1024] = "toto";
                        ImGui::Text("name"); ImGui::SameLine();
                        ImGui::InputText(label("", "gloglo"), tmp, 1024);
                        
                        if (ImGui::MenuItem(label("HexahedraLayer", mm_name))) {
                            auto group_opt = God::layers.find_group(mm_name);
                            int group_id = group_opt ? group_opt.value().id : God::layers.add_group(mm_name);
                            auto &layer = God::layers.add<HexahedraLayer>(tmp, group_id);
                            layer.init(mesh_id);
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
                if (!open) continue;
                for (auto& it : obj.attributes.points) 
                    add_attribute(
                        selected, 
                        mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::POINTS, .type = attribute_type(&*it.ptr)},
                        "points." + it.name
                    );
                for (auto& it : obj.attributes.cell_corners) 
                    add_attribute(
                        selected, 
                        mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::CELLCORNERS, .type = attribute_type(&*it.ptr)},
                        "corners." + it.name
                    );
                for (auto& it : obj.attributes.cell_facets) 
                    add_attribute(
                        selected, 
                        mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::CELLFACETS, .type = attribute_type(&*it.ptr)},
                        "facets." + it.name
                    );
                for (auto& it : obj.attributes.cells) 
                    add_attribute(
                        selected, 
                        mesh_id,
                        { .name = it.name, .primitive = AttributeBase::TYPE::CELLS, .type = attribute_type(&*it.ptr)},
                        "cells." + it.name
                    );
                add_mesh_layers(selected, {HEXAHEDRA, { mm_name, mesh_name }}, "hexahedra");
                ImGui::TreePop();
            }


            for (auto n : polyline2kill) God::xcf.kill_mesh(root().xcf[mm_name].polylines[n]);
            for (auto n : tri2kill) God::xcf.kill_mesh(root().xcf[mm_name].triangles[n]);
            for (auto n : hex2kill) God::xcf.kill_mesh(root().xcf[mm_name].hexahedra[n]);
            ImGui::TreePop();

        }
    }
    for (auto n : mm2kill) God::xcf.kill_multimesh(n);

    {// sync with property panel
        PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(ObjectId(root().panels["property_window"])));
        pan.layers.clear();
        for (auto sel : selected)if (sel.path == LAYER) pan.layers.push_back(sel);
    }



    ImGui::End();
}

void XCFExplorer::add_mesh_layers(ObjectIdSet& selected, ObjectId mesh_id, std::string gna){
    auto mm_name = mesh_id.names[0];
    
    for (int i = 0; i < God::layers.size(); i++)
        if (God::layers[i].mesh==mesh_id){
            ObjectId layer_id(LAYER, God::layers.ith_name(i));

            bool open_layer;
            {
                int colorid = 2;
                if (!selected.contains(layer_id)) colorid = 0;
                NodeColor ncol(colorid);
                open_layer = ImGui::TreeNodeEx(label(God::layers.ith_name(i), mm_name), flag(selected.contains(layer_id), true));
            }
            
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ATTR_INFO")) {
                    // TODO check size of payload is AttributeInfoPayload here
                    AttributeInfoPayload data;
                    std::memcpy(&data, payload->Data, sizeof(data));
                    AttributeInfo attr_info{};
                    attr_info.name = data.name;
                    attr_info.type = data.type;
                    attr_info.primitive = static_cast<AttributeBase::TYPE>(data.primitive);
                    Layer& layer = layer_id;
                    layer.bind_attribute(attr_info);
                }
                ImGui::EndDragDropTarget();
            }

            if (open_layer){
                if (ImGui::IsItemClicked()) switch_selection(selected, layer_id);
                ImGui::TreePop();
            }
        }
};
