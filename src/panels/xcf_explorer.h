struct XCFExplorer: public Panel {
	void generate_gui(){
		ImGui::SetNextWindowPos(ImVec2(10,10),ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(250,200),ImGuiCond_Always);

		ImGui::Begin("XCFViewer",nullptr,ImGuiWindowFlags_AlwaysAutoResize);

		std::vector<std::string> mm_to_kill;
		for(auto &[name,obj]:God::xcf){
			if (ImGui::CollapsingHeader(name.c_str())){
				if(ImGui::Button("Delete MultiMesh"))
					mm_to_kill.push_back(name);
			}
		}
		for(auto name:mm_to_kill)
			God::xcf.kill_multimesh(name);
		ImGui::End();
	}
};
