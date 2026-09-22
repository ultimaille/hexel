#pragma once

#include <map>
#include <fstream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ultimaille/all.h>

#include "basic.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"


// -------------------------------------------------------------------------------
//                                    God: you know... he knows
// -------------------------------------------------------------------------------

namespace InteractionMode{ struct AbstractMode; }

namespace God{
	// current interaction mode (controls everything: it is the entry point for different tools)
	extern InteractionMode::AbstractMode* root_mode;

	// datas actually manipulated by the modeler	
	extern XCF xcf;

	// input state
	extern MouseState mouse;
	extern KeyboardState keys;

	// event that occurred since last frame
	extern std::vector<Event> events;

	// the camera
	extern Camera camera;
	extern LayerManager layers;							// layers to be combined into the final rendering

	extern PanelManager panels;

	// API dependant 
	extern WindowContext context;
	extern ShaderManager shaders;
};


