#include "panels.h"

void PanelManager::show_gui() {
    ImGuiIO& io = ImGui::GetIO();
    
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        setup_dock_panels();

    for (auto& [name, obj] : *this)
        obj->generate_gui();
}

void PanelManager::setup_dock_panels() {
    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
    
    if (node)
        return;

    // Clear out existing layout
    ImGui::DockBuilderRemoveNode(dockspace_id);

    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());

    // Create a dock node for the right docked window
    ImGuiID left_bar0 = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 1.f, nullptr, &dockspace_id);
    ImGuiID left_bar1 = ImGui::DockBuilderSplitNode(left_bar0, ImGuiDir_Down, 0.8f, nullptr, &left_bar0);
    ImGuiID left_bar2 = ImGui::DockBuilderSplitNode(left_bar1, ImGuiDir_Down, 0.6f, nullptr, &left_bar1);
    ImGuiID left_bar3 = ImGui::DockBuilderSplitNode(left_bar2, ImGuiDir_Down, 0.5f, nullptr, &left_bar2);

    ImGui::DockBuilderDockWindow("XCFViewer", left_bar0);
    ImGui::DockBuilderDockWindow("LayersConfig", left_bar1);
    ImGui::DockBuilderDockWindow("Properties", left_bar2);
    ImGui::DockBuilderDockWindow("Mode", left_bar3);

    ImGui::DockBuilderFinish(dockspace_id);
}