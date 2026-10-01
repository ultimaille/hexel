#pragma once
#include "imgui_internal.h"
#include "core.h"

struct Panel{
    virtual ~Panel() = default;
    virtual void generate_gui()=0;
};

struct PanelManager: private Registry<Panel> {
    int size()                                { return Registry<Panel>::size(); }
    Panel& operator[](int i)                  { return Registry<Panel>::operator[](i); }
    Panel& operator[](std::string s)          { return Registry<Panel>::operator[](s); }
    std::string ith_name(int i)               { return items[i].name; }
    void swap(int i, int j)                   { std::swap(items[i], items[j]); }
    template<class T> T& add(std::string str) { return emplace_back<T>(str); }
    bool contains(std::string s)              { return Registry<Panel>::contains(s); }

    void show_gui(){
        setup_dock_panels();

        for(auto& [name,obj] : *this)
            obj->generate_gui();

    }

    void setup_dock_panels() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

            static auto first_time = true;
            if (first_time) {
                first_time = false;
                // Clear out existing layout
                ImGui::DockBuilderRemoveNode(dockspace_id);

                ImGuiID top_bar_node = ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_NoTabBar);
                ImGui::DockBuilderSetNodeSize(top_bar_node, ImVec2(viewport->WorkSize.x, 30.0f)); // Thin top 

                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());

                // get id of main dock space area
                ImGuiID dockspace_main_id = dockspace_id;
                // Create a dock node for the right docked window
                ImGuiID left_bar0 = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Left, 1.f, nullptr, &dockspace_main_id);
                // ImGuiID toolBar = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Right, 1.f, nullptr, &dockspace_main_id);
                // ImGuiID botBar = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Down, 0.2f, nullptr, &dockspace_main_id);
                ImGuiID left_bar1 = ImGui::DockBuilderSplitNode(left_bar0, ImGuiDir_Down, 0.25f, nullptr, &left_bar0);
                ImGuiID left_bar2 = ImGui::DockBuilderSplitNode(left_bar1, ImGuiDir_Down, 0.25f, nullptr, &left_bar1);

                ImGui::DockBuilderDockWindow("XCFViewer", left_bar0);
                ImGui::DockBuilderDockWindow("LayersConfig", left_bar1);
                ImGui::DockBuilderDockWindow("Properties", left_bar2);

                ImGui::DockBuilderFinish(dockspace_id);
            }
        }
    }
};