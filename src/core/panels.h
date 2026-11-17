#pragma once
#include "imgui_internal.h"
#include "core.h"
#include "registry.h"

struct Panel{
    virtual ~Panel() = default;
    virtual void generate_gui() = 0;
};

struct PanelManager : private Registry<Panel> {
    int size()                                { return Registry<Panel>::size(); }
    Panel& operator[](int i)                  { return Registry<Panel>::operator[](i); }
    Panel& operator[](std::string s)          { return Registry<Panel>::operator[](s); }
    std::string ith_name(int i)               { return items[i].name; }
    void swap(int i, int j)                   { std::swap(items[i], items[j]); }
    template<class T> T& add(std::string str) { return emplace_back<T>(str); }
    bool contains(std::string s)              { return Registry<Panel>::contains(s); }

    void show_gui();
    void setup_dock_panels();
};
