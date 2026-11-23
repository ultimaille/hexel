#pragma once
#include <modes/X/abstract_mode.h>
namespace ModeX{
    struct RootMode;

    struct StartingMode : public AbstractMode{
        StartingMode(RootMode& mode);
        virtual std::string name(){ return "Start"; }

        void define_gui();
        void handle(Event event);
        void import_new_project_from_CAD(std::string filename);
    };
};