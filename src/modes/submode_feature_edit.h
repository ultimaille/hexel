#include <modes/mode_hex_modeler.h>




struct FeatureEditMode : public HexModelerMode::SubMode{

    MouseReactMultiSelector multi_selector_react_left;
    MouseReactMultiSelector multi_selector_react_right;

    FeatureEditMode(HexModelerMode& mode);
    void activate();
    void define_gui();
    void handle(Event event);
};
