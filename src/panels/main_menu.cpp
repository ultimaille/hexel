#include "main_menu.h"
#include <core/all.h>
#include <misc/purgatory.h>

void MainMenu::generate_gui() {
    static std::string path; 

    bool load_mm = false;
    bool import_mesh = false;
    if (ImGui::BeginMainMenuBar()){
        if (ImGui::BeginMenu("Files")){

            if (ImGui::MenuItem("Load MultiMesh", NULL)) load_mm = true;
            if (ImGui::MenuItem("Import .geogram", NULL))import_mesh = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Modes")){
            ImGui::SeparatorText("Current mode");
            for (int i = 0; i < God::modes.size(); i++)
                if (ImGui::MenuItem(God::modes.ith_name(i).c_str(), NULL, God::modes.current_mode == i))
                    God::modes.current_mode = i;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (load_mm){ ImGui::OpenPopup("MMFilePopup"); load_mm = false; }
    if (FilePopup("MMFilePopup", std::string(TEST_INPUT_DIR), path, { ".mm" }))
        if (!path.empty()) load_mm_with_default_layers(path.substr(0, path.size() - 3));

    if (import_mesh){ ImGui::OpenPopup("ImportFilePopup"); import_mesh = false; }
    if (FilePopup("ImportFilePopup", std::string(TEST_INPUT_DIR), path, { ".geogram",".mesh",".meshb" }))
        if (!path.empty()) load_mm_with_default_layers(path.substr(0));
}
