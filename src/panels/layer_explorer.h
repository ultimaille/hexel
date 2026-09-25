struct LayerExplorer: public Panel {
	void generate_gui(){
		ImGui::SetNextWindowPos(ImVec2(10,210),ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(250,400),ImGuiCond_Always);
		ImGui::Begin("LayersConfig",nullptr,ImGuiWindowFlags_AlwaysAutoResize);

		for(auto &[name,obj]:God::layers){
			if(ImGui::TreeNode(name.c_str())){
				obj->generate_gui(name);
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
};