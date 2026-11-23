#pragma once
#include <modes/submode_X.h>
struct ModeX;
struct SubModeBasicEdit;


struct InteractorMoveVertex : public Interactor {
    InteractorMoveVertex(SubModeBasicEdit& mode);
    void on_press(int button, vec2 p);
    void on_release(int button, vec2 p);
    void generate_gui();
    void reset();

    Picker picker;
    SubModeBasicEdit& mode;
    int moving_vertex;
};



struct InteractorKillHex : public Interactor {
    InteractorKillHex(SubModeBasicEdit& mode);
    void on_click(int button, vec2 p);

    SubModeBasicEdit& mode;
};




struct InteractorFeedBack{
    void activate();
    void desactivate();
};


struct InteractorCreateHex: public Interactor {
    struct FeedBack : public InteractorFeedBack{
        void add_point(vec3 v0);
        void pop_point();
    } ;
    InteractorCreateHex(SubModeBasicEdit& mode);
    void on_click(int button, vec2 p);
    void handle(Event event);
    void reset();
    void generate_gui();
    inline bool in_progess(){ return !pts.empty(); }

    SubModeBasicEdit& mode;
    FeedBack feedback;
    std::vector<vec3> pts;
};

struct SubModeBasicEdit : public SubModeX {
    inline void activate(){Log::add("BasicEditMode activated");}
    virtual std::string name(){ return "Basic"; }

    SubModeBasicEdit(ModeX& mode);
    void handle(Event event);
    
    void define_gui();
    bool merge_vertices(int v0, int v1);
    InteractorKillHex interactor_kill_kex;
    InteractorMoveVertex interactor_move_vertex;
    InteractorCreateHex interactor_create_hex;
    InteractorTrackBallCamera interactor_camera;
    InteractorLayerSelection interactor_layer_selection;
};

