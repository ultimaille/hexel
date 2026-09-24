#pragma once

#include "basic.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "window.h"
#include "event.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"

namespace InteractionMode { struct AbstractMode; }

namespace God {
    extern InteractionMode::AbstractMode* root_mode; // TODO: we need to uniformize this with camera. Ptr or ref wrapper?
    extern XCF xcf;
    extern MouseState mouse;
    extern KeyboardState keys;
    extern EventManager events;
    extern Camera camera;
    extern LayerManager layers;
    extern PanelManager panels;
    extern WindowContext context;
    extern ShaderManager shaders;
};

