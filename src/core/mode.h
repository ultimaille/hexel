#pragma once
#include "event.h"
#include <iostream>

struct MouseReact{
    virtual void on_wheel(double v) {}
    virtual void on_click(int button, vec2 p){}
    virtual void on_press(int button, vec2 p) {}
    virtual void on_release(int button, vec2 p) {}
    virtual void on_drag(int button, vec2 a, vec2 b) {}
    int filter = 0;
};

struct Mode{
    enum Filter {
        ctrl_pressed = 1,
        shift_pressed = 2,
    };
    Registry<MouseReact> mouse_react;
    virtual void define_gui() = 0;
    virtual void handle(Event event) = 0;
    void handle_mouse(Event event);
};





struct MouseReactTrackBallCamera : public MouseReact{
    MouseReactTrackBallCamera(int filter = 0) {
        MouseReact::filter = filter;
    }
    void on_wheel(double v);
    void on_click(int button, vec2 p){}
    void on_press(int button, vec2 p){}
    void on_release(int button, vec2 p){}
    void on_drag(int button, vec2 a, vec2 b);
};



struct DefaultMode : public  Mode{
    DefaultMode() {
        mouse_react.emplace_back<MouseReactTrackBallCamera>("camera").filter = 0;
    }
    void define_gui()           { }
    void handle(Event event)    { handle_mouse(event); }
};


struct ModeManager : private Registry<Mode> {
    int size()                                  { return Registry<Mode>::size(); }
    Mode& operator[](int i)                     { return Registry<Mode>::operator[](i); }
    Mode& operator[](std::string s)             { return Registry<Mode>::operator[](s); }
    bool contains(std::string s)                { return Registry<Mode>::contains(s); }
    int find(std::string str)                   { return Registry<Mode>::find(str); }
    std::string ith_name(int i)                 { return items[i].name; }
    void swap(int i, int j)                     { std::swap(items[i], items[j]); }
    template<class T> T& add(std::string str)   { return emplace_back<T>(str); }

    int current_mode = 0;
    inline void define_gui()        { (*this)[current_mode].define_gui(); }
    inline void handle(Event event) { (*this)[current_mode].handle(event); };
};

