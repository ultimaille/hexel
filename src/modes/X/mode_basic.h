#pragma once
#include <modes/X/abstract_mode.h>
#include <modes/X/feedback_3d.h>
#include <modes/X/cmd.h>


namespace ModeX{
    struct RootMode;
    struct SubModeBasic;
    ENUM(QuadToHexStrategy, OFFSET_PROP_TO_QUAD, RAY_TRACE, FRAME_FIELD);


    struct BasicInteractor : public Interactor{
        BasicInteractor(SubModeBasic& mode);
        bool is_locked();
        bool have_lock();
        void get_lock();
        void release_lock();
        SubModeBasic& mode;
    };

    struct InteractorMoveVertex : public BasicInteractor {
        InteractorMoveVertex(SubModeBasic& mode);
        void on_press(int button, vec2 p);
        void on_release(int button, vec2 p);
        void generate_gui();
        void reset();

        Picker picker;
        int moving_vertex;
    };



    struct InteractorKillHex : public BasicInteractor {
        InteractorKillHex(SubModeBasic& mode);
        void on_click(int button, vec2 p);
    };


    struct InteractorCreateHex : public BasicInteractor {
        InteractorCreateHex(SubModeBasic& mode);
        void on_click(int button, vec2 p);
        void handle(Event event);
        void reset();
        void generate_gui();
        inline bool in_progess(){ return !pts.empty(); }
        void update_feedback();

        FeedBack3D  feedback;
        std::vector<vec3> pts;
    };




    struct SubModeBasic : public AbstractMode {
        inline void activate(){ Log::add("BasicEditMode activated"); }
        virtual std::string name(){ return "Basic"; }

        SubModeBasic(RootMode& mode);
        void handle(Event event);

        void define_gui();

        InteractorKillHex interactor_kill_kex;
        InteractorMoveVertex interactor_move_vertex;
        InteractorCreateHex interactor_create_hex;

        Interactor* has_lock=NULL;

        bool drop_vertices_on_geometry=true;
        
        QuadToHexStrategy::Choice quad_to_hex_strategy = QuadToHexStrategy::RAY_TRACE;
                     
    };

}