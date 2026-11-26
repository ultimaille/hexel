#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/X/root_mode.h>
#include <modes/X/mode_start.h>
#include <modes/X/mode_layer_split.h>
#include <modes/X/mode_feature.h>
#include <modes/X/mode_basic.h>



//#define WORKING_DIR std::string("/home/ssloy/+/hexel/build/NICO/")
#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")
#define TEST_INPUT_DIR std::string("C:\\NICO\\data\\preprocessed\\hexel\\")


namespace ModeX{


    void RootMode::update(){
        //static auto t = std::chrono::high_resolution_clock::now();
        //auto nt = std::chrono::high_resolution_clock::now();
        //if (hex_state == RootMode::ALL_OK)
        //    God::context.render_target.background_color = vec4(0, 0, 0, 1.);
        //else {
        //    double oscillation = 1.2 + std::cos(double(duration_cast<std::chrono::milliseconds>(nt - t).count()) / 50.);
        //    oscillation *= .3;
        //    if (hex_state == RootMode::TOPO_OK)
        //        God::context.render_target.background_color = .4 * vec4(.5 * oscillation, .5 * oscillation, 0, 1);
        //    else if (hex_state == RootMode::GEOM_OK)
        //        God::context.render_target.background_color = .4 * vec4(.5 * oscillation, .1 * oscillation, 0, 1);
        //    else
        //        God::context.render_target.background_color = .4 * vec4(oscillation, 0, 0, 1);
        //}
        God::context.render_target.background_color = vec4(0, 0, 0, 1.);
        sub_mode().update();
    }




    void RootMode::define_gui(){

        ImGui::Begin("ModeX", nullptr);
        //if (ImGui::Button("Backup")) scene().backup();
        //ImGui::SameLine();
        if (ImGui::Button("Undo")) scene().undo();
        ImGui::SameLine();
        if (ImGui::Button("Reset GUI"))  scene().restore_default_scene_rendering();
        ImGui::SameLine();
        if (ImGui::Button("SAVE"))  scene().save_project(project_directory+"/");
        ImGui::Separator();

        const int nmaxsubmode = 10;
        static std::vector<std::string> mode_names;
        if (mode_names.empty()) for (auto m : sub_modes)mode_names.push_back(m->name());
        um_assert(mode_names.size() < nmaxsubmode);
        const char* str[nmaxsubmode] = { NULL };
        FOR(i, mode_names.size())str[i] = mode_names[i].c_str();

        int nv_active_mode_id = active_mode_id;
        if (ImGui::Combo("SubMode", &nv_active_mode_id, str, mode_names.size())){
            sub_mode().desactivate();
            active_mode_id = nv_active_mode_id;
            sub_mode().activate();
        }
        ImGui_help("Choose the current submode\n" "hopefully you can go there");
        sub_mode().define_gui();
        ImGui::End();
    }




    void RootMode::handle(Event event) {
        Mode::handle(event);
        sub_mode().handle(event);                                                  
    };





    RootMode::RootMode() :sub_mode_start(*this), sub_mode_basic_edit(*this) {
        working_directory = WORKING_DIR;
        project_directory = WORKING_DIR;
        CAD_file = std::string(TEST_INPUT_DIR) + "B1.geogram";
        undo_stack_size = 5;
        active_mode_id = 0;
        sub_modes.push_back(&sub_mode_start);
        sub_modes.push_back(&sub_mode_basic_edit);

        if (0){
            project_directory = WORKING_DIR + "B1";
            sub_mode_start.import_new_project_from_CAD(CAD_file);
            active_mode_id = 1;
            scene().load_project(project_directory);
        }
        else
            active_mode_id = 0;
    }

}