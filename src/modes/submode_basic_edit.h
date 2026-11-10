#include <modes/mode_hex_modeler.h>


struct BasicEditMode;
struct MouseReactKillHex : public MouseReact {
    MouseReactKillHex(BasicEditMode& mode);
    BasicEditMode& mode;
    void on_click(int button, vec2 p);
};


struct InteractorFeedBack{
    void activate(vec3 v0);
    void add_point(vec3 v0);
    void desactivate();
};


struct MouseReactCreateHex: public MouseReact {
    MouseReactCreateHex(BasicEditMode& mode);
    BasicEditMode& mode;
    InteractorFeedBack feedback;
    //std::vector<int> vertices_stack;
    std::vector<vec3> pts;
    void on_click(int button, vec2 p);
    void handle(Event event);
    void reset();
    void generate_gui();
};

struct BasicEditMode : public HexModelerMode::SubMode{
    MouseReactKillHex interactor_kill_kex;
    MouseReactCreateHex interactor_create_hex;
    MouseReactTrackBallCamera interactor_camera;
    MouseReactLayerSelection interactor_layer_selection;

    inline void activate(){}
    BasicEditMode(HexModelerMode& mode);
    void handle(Event event);
    void define_gui();
};

