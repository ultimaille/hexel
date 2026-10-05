#pragma once

#include "basic.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"
#include "mode.h"

struct EventManager;

namespace InteractionMode { struct AbstractMode; }

namespace God {
    extern Mode mode; 
    extern UM::XCF xcf;
    extern MouseState mouse;
    extern KeyboardState keys;
    extern EventManager events;
    extern Camera camera;
    extern LayerManager layers;
    extern PanelManager panels;
    extern WindowContext context;
    extern ShaderManager shaders;
};

