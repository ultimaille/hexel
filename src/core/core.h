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
    extern ModeManager modes; 
    extern UM::XCF xcf;
    extern MouseState mouse;
    extern KeyboardState keys;
    extern EventManager events;
    extern TrackBallCamera camera;
    extern LayerManager layers;
    extern PanelManager panels;
    extern WindowContext context;
    extern ShaderManager shaders;
};

