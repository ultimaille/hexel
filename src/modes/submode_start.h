#pragma once
#include <modes/submode_X.h>
struct ModeX;

struct StartingMode : public SubModeX{
    StartingMode(ModeX& mode);
    void define_gui();
    void handle(Event event);
    void import_new_project_from_CAD(std::string filename);
};
