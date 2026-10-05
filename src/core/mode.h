#pragma once
#include "event.h"
#include <iostream>

struct MouseReaction{
    virtual void on_wheel(double v) {}
    virtual void on_click(int button, vec2 p){}
    virtual void on_pressed(int button, vec2 p) {}
    virtual void on_release(int button, vec2 p) {}
    virtual void on_drag(int button, vec2 a, vec2 b) {}
    int filter = 0;
};

struct ModeInterface{
    std::vector<std::unique_ptr< MouseReaction>> mouse_reactions;
    virtual void define_gui() = 0;
    virtual void handle(Event event) = 0;
};





struct MouseReactionTrackBallCamera : public MouseReaction{
    MouseReactionTrackBallCamera(int filter = 0) {
        MouseReaction::filter = filter;
    }
    void on_wheel(double v);
    void on_click(int button, vec2 p){}
    void on_pressed(int button, vec2 p){}
    void on_release(int button, vec2 p){}
    void on_drag(int button, vec2 a, vec2 b);
};



struct DefaultMode : public  ModeInterface{

    void define_gui() { std::cerr << "impl->defined_gui()\n"; }
    void handle(Event event){ std::cerr << "impl->handle(event)\n"; }
};


struct Mode {
    std::unique_ptr<ModeInterface> impl = std::make_unique<DefaultMode>();
    inline void define_gui() { impl->define_gui(); }
    inline void handle(Event event){ impl->handle(event); };
};

