#pragma once
#include <core/all.h>
#include <modes/mode_hex_modeler.h>                                 
#include <modes/mode_test.h>
#include <modes/mode_test_2.h>
struct Config {

    static void init_modes() {
        //God::modes.add<HexEdit>("hexedit");
        //God::modes.add<LayerSplitMode>("layer_split");
        God::modes.add<HexModelerMode>("hex_modeler_mode");
        God::modes.switch_to("hex_modeler_mode");
        
        God::modes.add<BenjiMode>("benjimode");
    }

};