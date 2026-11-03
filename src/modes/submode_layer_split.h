#include <modes/mode_hex_modeler.h>



struct LayerSplitMode : public HexModelerMode::SubMode{

    MouseReactMultiSelector multi_selector_react_left;
    MouseReactMultiSelector multi_selector_react_right;

    void activate(){}

    LayerSplitMode(HexModelerMode& mode);

    bool remove_hex_layer(Hexahedra& hex, CellFacetAttribute<bool>& kill_layer);

    void handle(Event event);

    void define_gui();
};

