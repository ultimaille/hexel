#define _USE_MATH_DEFINES
#include <cmath>
#include "mode.h"
#include "core.h"

namespace God {
    UM::XCF xcf;
    LayerManager layers;
    ShaderManager shaders;
    Mode mode;
    MouseState mouse;
    KeyboardState keys;
    WindowContext context;
    Camera camera;
    EventManager events;
    PanelManager panels;
}

