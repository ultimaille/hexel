#pragma once
#include "misc/purgatory.h"
#include "algo.h"
#include <core/mode.h>
#include<modes/submode_hex_modeler.h>
#include<modes/submode_start.h>
#include<modes/submode_basic_edit.h>





struct HexModelerMode : public Mode{

    HexModelerMode();

    enum SubModes{START,BASIC,LAYER,FEATURE};
//    const char* sub_mode_names[4] = { "start","Basic","layer","feature" };
    const char* sub_mode_names[2] = { "start","Basic"};
    std::vector<HexModelerSubMode*> sub_modes;

   
    void set_active_mode(SubModes sub);

    inline HexModelerSubMode& sub_mode(){ return *sub_modes[active_mode_id]; }


    void header_gui_for_submodes();
    void define_gui();
    void handle(Event event);
    void reset_gui();

    StartingMode sub_mode_start;
    BasicEditMode sub_mode_basic_edit;

    int active_mode_id = 0;

    std::string working_directory;
    std::string project_directory;
    std::string CAD_file;
    int undo_begin = -1;
    int undo_stack_size = 5;
    int undo_id = -1;
};


