#include "core/events.h"
struct PropertyExplorer : public Panel {
    std::vector<Path> layers;
    void generate_gui();
};
