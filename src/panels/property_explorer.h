#include "core/event.h"
struct PropertyExplorer : public Panel {
    std::vector<ObjectId> layers;
    void generate_gui();
};
