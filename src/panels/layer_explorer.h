#pragma once
#include <core/panels.h>

struct LayerExplorer : public Panel {
    void generate_gui(){

        draw_layer_gui();
        draw_group_gui();
    }


    void draw_layer_gui();
    void draw_group_gui();

};
