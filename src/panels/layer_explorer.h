struct LayerExplorer: public Panel {
	void generate_gui(){
		ImGui::SetNextWindowPos(ImVec2(10,410),ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(250,700),ImGuiCond_Always);
		ImGui::Begin("LayersConfig",nullptr,ImGuiWindowFlags_AlwaysAutoResize);

		
		
		int drag_from = -1;
		int drop_to = -1;
		for (int i = 0; i < God::layers.size(); ++i){
			RenderLayer& layer = God::layers[i];

			layer.generate_gui(God::layers.items[i].name);

			// The widget is the drag source
			if (ImGui::BeginDragDropSource()){
				ImGui::SetDragDropPayload("WIDGET",&i,sizeof(i));
				ImGui::Text("Moving %s", God::layers.items[i].name);
				ImGui::EndDragDropSource();
			}

			// The same widget is the drop target
			if (ImGui::BeginDragDropTarget()){
				if (const ImGuiPayload* payload =ImGui::AcceptDragDropPayload("WIDGET")){
					drag_from = *(const int*)payload->Data;
					drop_to = i;
				}
				ImGui::EndDragDropTarget();
			}
		}
		if (drag_from != -1 || drop_to != -1) {
			while (drag_from < drop_to) {
				std::swap(God::layers.items[drag_from], God::layers.items[drag_from + 1]);
				drag_from++;
			}
			while (drag_from > drop_to) {
				std::swap(God::layers.items[drag_from], God::layers.items[drag_from - 1]);
				drag_from--;
			}
		}

		ImGui::End();
	}
};