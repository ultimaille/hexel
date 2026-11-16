#include "xcf_explorer.h"
#include <core/all.h>
#include "panels/property_explorer.h"

#include <layers/pointset_layer.h> 
#include <layers/triangle_layer.h> 
#include <layers/polyline_layer.h> 
#include <layers/hexahedra_layer.h> 
#include <layers/ssao.h>

void XCFExplorer::object_id_drag_source(Path id, std::string name) {
    if (ImGui::BeginDragDropSource()) {
        grad_and_drop_objectid = id;
        Path* ptr = &grad_and_drop_objectid;
        ImGui::SetDragDropPayload("OBJECTID", &ptr, sizeof(Path*));
        ImGui::Text("Moving %s", name.c_str());
        ImGui::EndDragDropSource();
    }
}

bool XCFExplorer::open_mesh(PathSet& selected, MeshPath mesh, std::string category) {
    const bool is_pointset = mesh ==mesh.parent().pointset;
    const std::string display =
        is_pointset ? category : category + "." + mesh.name();

    bool open;
    {
        NodeColor ncol(1);
        open = ImGui::TreeNodeEx(
            label(display, mesh.parent().name()),
            flag(selected.contains(mesh), false));
    }

    if (ImGui::IsItemClicked()) switch_selection(selected, mesh);
    object_id_drag_source(mesh, display);
    return open;
}

void XCFExplorer::add_attribute(PathSet& selected, MeshPath mesh_id, AttributePath attr_id, std::string name){
    bool open_attr;
    {
        NodeColor ncol(3);
        open_attr = ImGui::TreeNodeEx(label(name, mesh_id.parent().name()), flag(selected.contains(mesh_id), true));
        object_id_drag_source(attr_id, name);
    }
    if (open_attr){
        if (ImGui::IsItemClicked()) switch_selection(selected, attr_id);
        ImGui::TreePop();
    }
};

void XCFExplorer::generate_gui() {
    static PathSet selected([&](const Path& a, const Path& b) { return a.path < b.path; });

    {// sync with property panel
        PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(root.panels["property_window"]));
        std::vector<Path> to_kill;
        for (auto sel : selected) if (sel.has_prefix(root.layers)) to_kill.push_back(sel);
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
        MultiMeshPath mm_id = root.xcf[mm_name];
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
                    God::xcf.add_mesh(root.xcf[mm_name].polylines[tmp]);
                    ImGui::CloseCurrentPopup();
                }
                if (ImGui::MenuItem(label("Triangles", mm_name))){
                    God::xcf.add_mesh(root.xcf[mm_name].triangles[tmp]);
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
                    God::xcf.add_mesh(root.xcf[mm_name].hexahedra[tmp]);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }


        if (open_mm){
            // ==> pointset
            MeshPath mesh_id = root.xcf[mm_name].pointset;

            bool open_pointset = open_mesh(selected, mesh_id, "pointset");

            if (ImGui::BeginPopupContextItem(label("pointset_context", mm_name + "pointset"))) {

                if (ImGui::BeginMenu("create")) {
                    
                    static char tmp[1024] = "toto";
                    ImGui::Text("name"); ImGui::SameLine();
                    ImGui::InputText(label("", "gloglo"), tmp, 1024);
                    
                    if (ImGui::MenuItem(label("PointSetLayer", mm_name))) {
                        auto group_opt = God::layers.find_group(mm_name);
                        int group_id = group_opt ? group_opt.value().id : God::layers.add_group(mm_name);
                        auto &layer = God::layers.add<PointSetLayer>(tmp, group_id);
                        layer.init(static_cast<MeshPath>((Path)mesh_id));
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
//                      ObjectId(ObjectType::POINTSET_ATTR, { mm_name, attribute_type(&*it.ptr), "points", it.name }), 
                        root.xcf[mm_name].pointset.attr_points[it.name],
                        "points." + it.name
                    );
                }
                add_mesh_layers(selected, root.xcf[mm_name].pointset, "pointset");
                ImGui::TreePop();
            }






            // ==> polylines
            std::vector<std::string> polyline2kill;
            for (auto& [mesh_name, obj] : mm.polylines) {
                MeshPath mesh_id = root.xcf[mm_name].polylines[mesh_name];

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
                            layer.init(static_cast<MeshPath>((Path)mesh_id));
                            layer.set_default_values();
                            ImGui::CloseCurrentPopup();
                        }


                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
                if (!open) continue;

                for (auto& it : obj.attributes.points) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].polylines[mesh_name].attr_points[it.name],
//                  ObjectId(ObjectType::POLYLINES_ATTR, { mm_name, mesh_name ,attribute_type(&*it.ptr),"points",it.name }),
                    "points." + it.name);
                for (auto& it : obj.attributes.edges) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].polylines[mesh_name].attr_edges[it.name],
//                  ObjectId(ObjectType::POLYLINES_ATTR, { mm_name, mesh_name ,attribute_type(&*it.ptr),"edges",it.name }),
                    "edges." + it.name);

                add_mesh_layers(selected, root.xcf[mm_name].polylines[mesh_name], "polylines");
                ImGui::TreePop();
            }

            // TODO: can refactor duplicate (triangle / quads; tetrahedra / hexahedra), resolving some ObjectType at compile-time
            // But wait for new object id before doing this !
            // ==> triangles
            std::vector<std::string> tri2kill;
            for (auto& [mesh_name, obj] : mm.triangles) {

                MeshPath mesh_id = root.xcf[mm_name].triangles[mesh_name];

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
                            layer.init(static_cast<MeshPath>((Path)mesh_id));
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }

                if (!open) continue;
                for (auto& it : obj.attributes.points) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].triangles[mesh_name].attr_points[it.name],
                    "points." + it.name);
                for (auto& it : obj.attributes.corners) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].triangles[mesh_name].attr_corners[it.name],
                    "corners." + it.name);
                for (auto& it : obj.attributes.facets) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].triangles[mesh_name].attr_facets[it.name],
                    "facets." + it.name);
                add_mesh_layers(selected, root.xcf[mm_name].triangles[mesh_name], "triangles");
                ImGui::TreePop();

            }

            // // ==> quads
            // std::vector<std::string> quad2kill;
            // for (auto& [mesh_name, obj] : mm.quads) {
            //     bool open = open_mesh(selected, mm_name, mesh_name, QUADS, "quads");
            //     if (ImGui::BeginPopupContextItem(label("killquadpopup", mm_name + mesh_name))){
            //         if (ImGui::Button("Delete")){
            //             quad2kill.push_back(mesh_name);
            //             ImGui::CloseCurrentPopup();
            //         }
            //         ImGui::EndPopup();
            //     }
            //     if (!open) continue;
            //     for (auto& it : obj.attributes.points) add_attribute(selected, mesh_id,
            //         ObjectId(ObjectType::QUADS_ATTR, { mm_name, mesh_name, attribute_type(&*it.ptr),"points", it.name}),
            //         "points." + it.name);
            //     for (auto& it : obj.attributes.corners) add_attribute(selected, mesh_id,
            //         ObjectId(ObjectType::QUADS_ATTR, { mm_name, mesh_name, attribute_type(&*it.ptr),"corners", it.name}),
            //         "corners." + it.name);
            //     for (auto& it : obj.attributes.facets) add_attribute(selected, mesh_id,
            //         ObjectId(ObjectType::QUADS_ATTR, { mm_name, mesh_name, attribute_type(&*it.ptr),"facets", it.name}),
            //         "facets." + it.name);
            //     add_mesh_layers(selected, mm_name, mesh_name, QUADS, "quads");
            //     ImGui::TreePop();

            // }

            // // ==> tetrahedra
            // std::vector<std::string> tet2kill;
            // for (auto& [mesh_name, obj] : mm.tetrahedra) {
            //     bool open = open_mesh(selected, mm_name, mesh_name, TETRAHEDRA, "tetrahedra");
            //     if (ImGui::BeginPopupContextItem(label("killtetpopup", mm_name + mesh_name))){
            //         if (ImGui::Button("Delete")){
            //             tet2kill.push_back(mesh_name);
            //             ImGui::CloseCurrentPopup();
            //         }
            //         ImGui::EndPopup();
            //     }
            //     if (!open) continue;
            //     for (auto& it : obj.attributes.points) add_attribute(selected, mesh_id,
            //         ObjectId(ObjectType::TETRAHEDRA_ATTR, { mm_name, mesh_name ,attribute_type(&*it.ptr),"points",it.name }),
            //         "points." + it.name);
            //     for (auto& it : obj.attributes.cell_corners) add_attribute(selected, mesh_id,
            //         ObjectId(ObjectType::TETRAHEDRA_ATTR, { mm_name, mesh_name ,attribute_type(&*it.ptr),"corners",it.name }),
            //         "corners." + it.name);
            //     for (auto& it : obj.attributes.cell_facets) add_attribute(selected, mesh_id,
            //         ObjectId(ObjectType::TETRAHEDRA_ATTR, { mm_name, mesh_name ,attribute_type(&*it.ptr),"facets",it.name }),
            //         "facets." + it.name);
            //     for (auto& it : obj.attributes.cells) add_attribute(selected, mesh_id,
            //         ObjectId(ObjectType::TETRAHEDRA_ATTR, { mm_name, mesh_name ,attribute_type(&*it.ptr),"cells",it.name}),
            //         "cells." + it.name);
            //     add_mesh_layers(selected, mm_name, mesh_name, TETRAHEDRA, "tetrahedra");
            //     ImGui::TreePop();
            // }

            // ==> hexahedra
            std::vector<std::string> hex2kill;
            for (auto& [mesh_name, obj] : mm.hexahedra) {
                MeshPath mesh_id = root.xcf[mm_name].hexahedra[mesh_name];
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
                            layer.init(static_cast<MeshPath>((Path)mesh_id));
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }
                if (!open) continue;
                for (auto& it : obj.attributes.points) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].hexahedra[mesh_name].attr_points[it.name],
                    "points." + it.name);
                for (auto& it : obj.attributes.cell_corners) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].hexahedra[mesh_name].attr_corners[it.name],
                    "corners." + it.name);
                for (auto& it : obj.attributes.cell_facets) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].hexahedra[mesh_name].attr_facets[it.name],
                    "facets." + it.name);
                for (auto& it : obj.attributes.cells) add_attribute(selected, mesh_id,
                        root.xcf[mm_name].hexahedra[mesh_name].attr_cells[it.name],
                    "cells." + it.name);
                add_mesh_layers(selected, root.xcf[mm_name].hexahedra[mesh_name], "hexahedra");
                ImGui::TreePop();
            }


            for (auto n : polyline2kill) God::xcf.kill_mesh(root.xcf[mm_name].polylines[n]);
            for (auto n : tri2kill) God::xcf.kill_mesh(root.xcf[mm_name].triangles[n]);
            // for (auto n : quad2kill) God::xcf.kill_mesh(ObjectId(ObjectType::QUADS, {mm_name, n}));
            // for (auto n : tet2kill) God::xcf.kill_mesh(ObjectId(ObjectType::TETRAHEDRA, {mm_name, n}));
            for (auto n : hex2kill) God::xcf.kill_mesh(root.xcf[mm_name].hexahedra[n]);
            ImGui::TreePop();

        }
    }
    for (auto n : mm2kill) God::xcf.kill_multimesh(n);

    {// sync with property panel
        PropertyExplorer& pan = dynamic_cast<PropertyExplorer&> (static_cast<Panel&>(root.panels["property_window"]));
        pan.layers.clear();
        for (auto sel : selected)if (sel.has_prefix(root.layers)) pan.layers.push_back(sel);
    }



    ImGui::End();
}

void XCFExplorer::add_mesh_layers(PathSet& selected, MeshPath mesh_id, std::string gna){
    auto mm_name = mesh_id.parent().name();
    
    for (int i = 0; i < God::layers.size(); i++)
        if (God::layers[i].mesh== mesh_id){
            Path layer_id = root.layers[God::layers.ith_name(i)];

            bool open_layer;
            {
                int colorid = 2;
                if (!selected.contains(layer_id)) colorid = 0;
                NodeColor ncol(colorid);
                open_layer = ImGui::TreeNodeEx(label(God::layers.ith_name(i), mm_name), flag(selected.contains(layer_id), true));
            }
            AttributePath attr_id;
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OBJECTID")) {
                    attr_id = **(const AttributePath**)payload->Data;
                    Layer& layer = layer_id;
                    layer.bind_attribute(static_cast<AttributePath>((Path)attr_id));
                }
                ImGui::EndDragDropTarget();
            }

            if (open_layer){
                if (ImGui::IsItemClicked()) switch_selection(selected, layer_id);
                ImGui::TreePop();
            }
        }
};
