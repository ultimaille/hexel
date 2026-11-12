#pragma once
#include <modes/submode_hex_modeler.h>
struct HexModelerMode;

struct StartingMode : public HexModelerSubMode{
    StartingMode(HexModelerMode& mode);
    void define_gui();
    void handle(Event event);
    void import_new_project_from_CAD(std::string filename);
};
