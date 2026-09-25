#include "core.h"

Picker::Picker(vec4 rect){
	#ifdef ACTIVATE
	God::layers.read_framebuffer(layer_data, rect, 1);
	God::layers.read_framebuffer(primitive_data, rect, 2);
	#endif
}

Picker::Picker(){
	#ifdef ACTIVATE
	w = God::context.window.render_target.width;
	h = God::context.window.render_target.height;
	#else
	const int w = 1;
	const int h = 1;
	#endif
	Picker(vec4{0, 0, w, h});
}