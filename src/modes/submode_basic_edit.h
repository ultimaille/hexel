#pragma once
#include <modes/submode_hex_modeler.h>
struct HexModelerMode;

struct BasicEditMode;
struct MouseReactKillHex : public MouseReact {
    MouseReactKillHex(BasicEditMode& mode);
    BasicEditMode& mode;
    void on_click(int button, vec2 p);
};


struct InteractorFeedBack{
    void activate(vec3 v0);
    void add_point(vec3 v0);
    void pop_point();
    void desactivate();
};


struct MouseReactCreateHex: public MouseReact {
    MouseReactCreateHex(BasicEditMode& mode);
    BasicEditMode& mode;
    InteractorFeedBack feedback;
    std::vector<vec3> pts;
    void on_click(int button, vec2 p);
    void handle(Event event);
    void reset();
    void generate_gui();
};

struct BasicEditMode : public HexModelerSubMode {
    MouseReactKillHex interactor_kill_kex;
    MouseReactCreateHex interactor_create_hex;
    MouseReactTrackBallCamera interactor_camera;
    MouseReactLayerSelection interactor_layer_selection;

    inline void activate(){
        Log::add("BasicEditMode activated");
    }
    BasicEditMode(HexModelerMode& mode);
    void handle(Event event);
    void define_gui();
};

