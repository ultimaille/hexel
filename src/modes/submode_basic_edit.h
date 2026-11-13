#pragma once
#include <modes/submode_hex_modeler.h>
struct HexModelerMode;
struct BasicEditMode;


struct InteractorMoveVertex : public Interactor {
    InteractorMoveVertex(BasicEditMode& mode);
    void on_press(int button, vec2 p);
    void on_release(int button, vec2 p);
    void generate_gui();
    void reset();

    Picker picker;
    BasicEditMode& mode;
    int moving_vertex;
};



struct InteractorKillHex : public Interactor {
    InteractorKillHex(BasicEditMode& mode);
    void on_click(int button, vec2 p);

    BasicEditMode& mode;
};




struct InteractorFeedBack{
    void activate(vec3 v0);
    void add_point(vec3 v0);
    void pop_point();
    void desactivate();
};


struct InteractorCreateHex: public Interactor {
    InteractorCreateHex(BasicEditMode& mode);
    void on_click(int button, vec2 p);
    void handle(Event event);
    void reset();
    void generate_gui();

    BasicEditMode& mode;
    InteractorFeedBack feedback;
    std::vector<vec3> pts;
};

struct BasicEditMode : public HexModelerSubMode {
    inline void activate(){Log::add("BasicEditMode activated");}
    BasicEditMode(HexModelerMode& mode);
    void handle(Event event);
    void define_gui();

    InteractorKillHex interactor_kill_kex;
    InteractorMoveVertex interactor_move_vertex;
    InteractorCreateHex interactor_create_hex;
    InteractorTrackBallCamera interactor_camera;
    InteractorLayerSelection interactor_layer_selection;
};

