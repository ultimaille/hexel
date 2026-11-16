#pragma once
#include "events.h"
#include "registry.h"
#include <iostream>
#include <set>

struct Interactor{

    // high level events
    virtual void on_wheel(double v) {}
    virtual void on_click(int button, vec2 p){}
    virtual void on_press(int button, vec2 p) {}
    virtual void on_release(int button, vec2 p) {}
    virtual void on_drag(int button, vec2 a, vec2 b) {}

    // gather informations to manage the life cycle
    virtual void handle(Event event){}

    // to have trace on screen
    virtual void draw_data_gui(){}
    virtual void generate_gui(){}


    // manage switch key conditions
    bool key_state_is_OK();

    bool ctrl_must_be_down=false;
    bool shift_must_be_down = false;
    bool ctrl_must_be_up = false;
    bool shift_must_be_up = false;
};

struct Mode{
    std::vector<Interactor*> interactors;
    virtual void activate(){}
    virtual void desactivate(){}
    virtual void define_gui() {}
    virtual void handle(Event event);
    void draw_data_gui();
    void generate_interactor_gui();
};





struct InteractorTrackBallCamera : public Interactor{
    void on_wheel(double v);
    void on_drag(int button, vec2 a, vec2 b);
};


struct DefaultMode : public  Mode{
    InteractorTrackBallCamera interactor_camera;
    DefaultMode();
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

