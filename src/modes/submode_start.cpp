#include <modes/submode_start.h>




    StartingMode::StartingMode(HexModelerMode& mode) : HexModelerMode::SubMode(mode){}
    void StartingMode::define_gui() {
        ImGui::Begin("HexModelerMode", nullptr);
        if (ImGui::Button("Import From CAD")) ImGui::OpenPopup("CAD_file");
        if (FilePopup("CAD_file", std::filesystem::path(mode.CAD_file).parent_path().string(), mode.CAD_file, { ".geogram",".meshb",".mesh" }))
            if (!mode.CAD_file.empty()) {
                mode.project_directory = mode.working_directory + std::filesystem::path(mode.CAD_file).stem().string() + "\\";
                std::filesystem::create_directory(mode.project_directory);
                mode.import_new_project_from_CAD(mode.CAD_file);
                mode.load_project();
            }

        if (ImGui::Button("Load project")) ImGui::OpenPopup("Load project");
        if (FilePopup("Load project", mode.project_directory, mode.project_directory, { "*" }))
            if (!mode.project_directory.empty()){
                mode.project_directory = mode.project_directory + "\\";
                mode.load_project();
            }
        ImGui::End();
    }


    void StartingMode::handle(Event event) {
        // forward mouse events to mouse reacts 
            mode.handle_camera_and_layerselect(event);
    }