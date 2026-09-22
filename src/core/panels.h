#pragma once
#include "core.h"

struct Panel{
    virtual ~Panel() = default;
    virtual void generate_gui()=0;
};

struct PanelManager: public Registry<Panel> {
    void show_gui(){
        for(auto& [name,obj] : *this)
            obj->generate_gui();
    }
};


