#define _USE_MATH_DEFINES
#include <cmath>
#include "mode.h"
#include "core.h"

namespace God {
    UM::XCF xcf;
    LayerManager layers;
    ShaderManager shaders;
    ModeManager modes;
    MouseState mouse;
    KeyboardState keys;
    WindowContext context;
    TrackBallCamera camera;
    EventManager events;
    PanelManager panels;
}

