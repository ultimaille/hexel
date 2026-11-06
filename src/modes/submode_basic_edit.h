#include <modes/mode_hex_modeler.h>



struct BasicEditMode;
struct MouseReactKillHex : public MouseReact {
    MouseReactKillHex(BasicEditMode& mode);
    BasicEditMode& mode;
    void on_click(int button, vec2 p);
};

struct MouseReactCreateHex: public MouseReact {
    MouseReactCreateHex(BasicEditMode& mode);
    BasicEditMode& mode;
    std::vector<int> vertices_stack;
    void on_click(int button, vec2 p);
};



struct BasicEditMode : public HexModelerMode::SubMode{
    MouseReactKillHex react_kill_hex;
    MouseReactCreateHex react_create_hex;

    inline void activate(){}

    BasicEditMode(HexModelerMode& mode);

    void handle(Event event);

    void define_gui();
};

