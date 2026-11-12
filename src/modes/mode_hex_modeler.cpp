#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/mode_hex_modeler.h>
#include <modes/submode_start.h>
#include <modes/submode_layer_split.h>
#include <modes/submode_feature_edit.h>
#include <modes/submode_basic_edit.h>



//#define WORKING_DIR std::string("/home/ssloy/+/hexel/build/NICO/")
#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")



void HexModelerMode::reset_gui(){
    sub_mode().load_project(project_directory);
}



void HexModelerMode::header_gui_for_submodes(){
    if (ImGui::Button("Backup")) sub_mode().backup();
    ImGui::SameLine();
    if (ImGui::Button("Undo")) sub_mode().undo();
    ImGui::SameLine();
    if (ImGui::Button("Reset GUI")) reset_gui();
    ImGui::Separator();

    //const char* items[] = { "Load/Import", "BasicEdit", "LayerSplit", "CAD_feature" };

    if (ImGui::Combo("SubMode", &active_mode_id, sub_mode_names, IM_COUNTOF(sub_mode_names))){

        if (active_mode_id == 0) set_active_mode(START);
        if (active_mode_id == 1) set_active_mode(BASIC);
        if (active_mode_id == 2) set_active_mode(LAYER);
        if (active_mode_id == 3) set_active_mode(FEATURE);
    }
    
    ImGui_help("Choose the current submode\n" "hopefully you can go there");
}

void HexModelerMode::define_gui(){
    sub_mode().define_gui();
}


void HexModelerMode::handle(Event event) {
    Mode::handle(event);
    sub_mode().handle(event);
};

void HexModelerMode::set_active_mode(SubModes sub){
    int id = sub;
    um_assert(id != -1);
    sub_mode().desactivate();
    active_mode_id = id;
    sub_mode().activate();
}



HexModelerMode::HexModelerMode():sub_mode_start(*this),sub_mode_basic_edit(*this){
        working_directory = WORKING_DIR;
        project_directory = WORKING_DIR;
        CAD_file = std::string(TEST_INPUT_DIR) + "B1.geogram";
        undo_stack_size = 5;
    active_mode_id = 0;
    sub_modes.push_back(&sub_mode_start);
    sub_modes.push_back(&sub_mode_basic_edit);
    //sub_modes.emplace_back<StartingMode>("StartingMode", *this);
    //sub_modes.emplace_back<BasicEditMode>("BasicEditMode", *this);
    //modes.emplace_back<LayerSplitMode>("LayerSplitMode", *this);
    //modes.emplace_back<FeatureEditMode>("FeatureEditMode", *this);

    if (1){
        project_directory = WORKING_DIR + "B1";
        sub_mode_start.import_new_project_from_CAD(CAD_file);
        set_active_mode(BASIC);
        sub_mode().load_project(project_directory);
    }else
        set_active_mode(START);
}
