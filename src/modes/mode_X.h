#pragma once
#include "misc/purgatory.h"
#include "algo.h"
#include <core/mode.h>
#include<modes/submode_X.h>
#include<modes/submode_start.h>
#include<modes/submode_basic_edit.h>



//namespace ModeX{

    struct ModeX : public Mode{

        ModeX();
        virtual std::string name(){ return "modeX"; }

        // manage sub modes
        std::vector<SubModeX*> sub_modes;
        inline SubModeX& sub_mode(){ return *sub_modes[active_mode_id]; }

        StartingMode sub_mode_start;
        SubModeBasicEdit sub_mode_basic_edit;
        int active_mode_id = 0;


        void define_gui();
        void handle(Event event);
        void update();

        ModeXScene scene(){ return ModeXScene(*this); }


        // share data between submodes: hex validity + serialization parameters
        enum HexState{ TOPO_OK = 1, GEOM_OK = 2, ALL_OK = 3 };
        int hex_state = TOPO_OK + GEOM_OK;

        std::string working_directory;
        std::string project_directory;
        std::string CAD_file;

        int undo_begin = -1;
        int undo_stack_size = 5;
        int undo_id = -1;
    };

//};
