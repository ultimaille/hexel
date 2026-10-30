#include <modes/mode_hex_modeler.h>



struct MouseReactSplitAddFeature : public MouseReact {
    ObjectId pl_id;
    ObjectId pts_id;
    int org_vertex;
    void init(ObjectId p_pl, ObjectId p_pts);
    void on_click(int button, vec2 p);
    void define_gui();
};



struct FeatureEditMode : public HexModelerMode::SubMode{

    MouseReactMultiSelector multi_selector_react_left;
    MouseReactMultiSelector multi_selector_react_right;
    MouseReactSplitAddFeature split_add_react;

    FeatureEditMode(HexModelerMode& mode);
    void activate();
    void define_gui();
    void handle(Event event);
};
