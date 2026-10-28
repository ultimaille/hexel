#pragma once
#include <core/all.h>
#include <modes/mode_test.h>
#include <modes/mode_test_2.h>
#include <modes/mode_layer_split.h>
struct Config {

    static void init_modes() {
        God::modes.add<HexEdit>("hexedit");

        God::modes.add<LayerSplitMode>("layer_split");
        God::modes.add<HexModelerMode>("hexmodeler");
        
        God::modes.switch_to("hexmodeler");

        God::modes.add<BenjiMode>("benjimode");
    }

};