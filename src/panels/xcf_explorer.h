struct XCFExplorer: public Panel {
	void generate_gui(){
		ImGui::SetNextWindowPos(ImVec2(10,10),ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(250,400),ImGuiCond_Always);

		ImGui::Begin("XCFViewer",nullptr,ImGuiWindowFlags_AlwaysAutoResize);

		std::vector<std::string> mm_to_kill;
		for(auto &[mm_name,mm]:God::xcf){
			if (ImGui::CollapsingHeader(mm_name.c_str())) {

				auto imgui_str = [&](std::string str, std::string suffix ="") {return (str + "##" + mm_name+suffix).c_str();};

				if (ImGui::TreeNode(imgui_str("pointset" ))) {
					//... show shaders
					ImGui::TreePop();
				}
				if (!mm.polylines.empty()) if (ImGui::TreeNode(imgui_str("polylines"))) {
					//... show shaders
					ImGui::TreePop();
				}



				// show interface for the collection of triangles
				if (!mm.triangles.empty()) if (ImGui::TreeNode(imgui_str("triangles" ))) {
					std::vector<ObjectId> to_kill;
				

					for (auto& [tri_name, obj] : mm.triangles) {
						static bool closable_group = true; 
						ObjectId id({ chunk_xcf,mm_name,"triangles",tri_name });
						if (ImGui::CollapsingHeader(imgui_str(tri_name, "tri"), &closable_group)) {
							// render layers
							for (auto& [layer_name, layer] : God::layers) {
								if (layer->require(id)) {
									layer->generate_gui(layer_name);
								}
							}
						}

						if (ImGui::Button(imgui_str("kill odd triangles",tri_name))) {
							Triangles &tri = obj.mesh;
							std::vector<bool> to_kill(tri.nfacets(),false);
							for (auto f : tri.iter_facets()) to_kill[f] = (f % 2) == 0;
							tri.disconnect();
							tri.delete_facets(to_kill);
							tri.connect();
							id.emit(UPDATED);
						}

						// delete if the cross is pressed
						if (!closable_group) {
							to_kill.push_back(id);
							closable_group = true;
						}
					}
					for (auto id : to_kill) {
						id.show();
						God::xcf.kill_mesh(id);
					}















					{// create new triangles
						static char new_tri_name[64] = "newtri";
						if (ImGui::Button(imgui_str("Add new"))) {
							std::string s = new_tri_name;
							God::xcf.add_triangles(mm_name, s);
							ImGui::TreePop();
							break;
						}
						ImGui::SameLine(); ImGui::InputText(imgui_str("##edit"), new_tri_name, 64);
					}

					ImGui::TreePop();
				}
				if (!mm.quads.empty()) if (ImGui::TreeNode(imgui_str("quads"))) {
					//... show shaders
					ImGui::TreePop();
				}


				if(ImGui::Button(imgui_str("Delete MultiMesh")))
					mm_to_kill.push_back(mm_name);

			}
		}
		for(auto name:mm_to_kill)
			God::xcf.kill_multimesh(name);
		ImGui::End();
	}
};
