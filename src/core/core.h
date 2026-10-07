#pragma once

#include "basic.h"


struct EventManager;
struct ModeManager;
namespace UM{
    struct XCF;
};
struct MouseState;
struct KeyboardState;
struct EventManager;
struct TrackBallCamera;
struct LayerManager;
struct PanelManager;
struct WindowContext;
struct ShaderManager;



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

