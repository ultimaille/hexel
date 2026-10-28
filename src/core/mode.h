#pragma once
#include "event.h"
#include <iostream>
#include <set>

struct MouseReact{

    virtual void on_wheel(double v) {}
    virtual void on_click(int button, vec2 p){}
    virtual void on_press(int button, vec2 p) {}
    virtual void on_release(int button, vec2 p) {}
    virtual void on_drag(int button, vec2 a, vec2 b) {}
};

struct Mode{
    virtual void activate(){}
    virtual void desactivate(){}
    virtual void define_gui() = 0;
    virtual void handle(Event event) = 0;
    void send_instruction_to_react(Event event, MouseReact &react);
};





struct MouseReactTrackBallCamera : public MouseReact{
    void on_wheel(double v);
    void on_click(int button, vec2 p){}
    void on_press(int button, vec2 p){}
    void on_release(int button, vec2 p){}
    void on_drag(int button, vec2 a, vec2 b);
};



struct DefaultMode : public  Mode{
    MouseReactTrackBallCamera camera_react;
    DefaultMode() {}
    void define_gui()           { }
    void handle(Event event)    { send_instruction_to_react(event, camera_react); }
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
    inline void switch_to(std::string s)        {
        int new_mode = find(s);
        um_assert(new_mode != -1);
        (*this)[current_mode].desactivate();
        current_mode = new_mode;
        (*this)[current_mode].activate();
    }
    inline void define_gui()        { (*this)[current_mode].define_gui(); }
    inline void handle(Event event) { (*this)[current_mode].handle(event); };
};

