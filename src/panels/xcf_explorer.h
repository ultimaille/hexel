struct XCFExplorer: public Panel {
	void generate_gui(){
		ImGui::SetNextWindowPos(ImVec2(10,10),ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(250,200),ImGuiCond_Always);


		ImGui::Begin("XCFViewer",nullptr,ImGuiWindowFlags_AlwaysAutoResize);


		std::vector<std::string> mm_to_kill;
		for(auto &[name,obj]:God::xcf){
			if(ImGui::TreeNode(name.c_str())){
				if(ImGui::Button("Delete MultiMesh"))
					mm_to_kill.push_back(name);
				ImGui::TreePop();
			}
		}
		for(auto name:mm_to_kill){
			God::events.push_back({Event::MM_REMOVED,name});
			God::xcf.erase(name);
		}
		ImGui::End();
	}
};
