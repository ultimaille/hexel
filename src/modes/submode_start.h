#include <modes/mode_hex_modeler.h>




struct StartingMode : public HexModelerMode::SubMode{
    StartingMode(HexModelerMode& mode);
    void define_gui();
    void handle(Event event);
};
