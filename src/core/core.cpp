#define _USE_MATH_DEFINES
#include "core.h"
#include <cmath>

namespace God {
    XCF xcf;
    LayerManager layers;
    ShaderManager shaders;
    InteractionMode::AbstractMode* root_mode;
    MouseState mouse;
    KeyboardState keys;
    WindowContext context;
    Camera camera;
    std::vector<Event> events;
    PanelManager panels;
}



