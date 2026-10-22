#pragma once
#include <core/all.h>
#include <modes/mode_test.h>
#include <modes/mode_test_2.h>

struct Config {

    static void init_modes() {
        God::modes.add<DefaultMode>("default");
    }

};