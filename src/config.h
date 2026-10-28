#pragma once
#include <core/all.h>
#include <modes/mode_test.h>
#include <modes/mode_test_2.h>

struct Config {

    static void init_modes() {
        God::modes.add<HexEdit>("hexedit");

        // God::modes.add<LayerSplitMode>("layer_split"); 
        // God::modes.current_mode = 2;

        God::modes.add<BenjiMode>("benjimode");
        God::modes.current_mode = 2;
    }

};