#include <modes/mode_hex_modeler.h>



struct BasicEditMode : public HexModelerMode::SubMode{


    inline void activate(){}

    BasicEditMode(HexModelerMode& mode);

    void handle(Event event);

    void define_gui();
};

