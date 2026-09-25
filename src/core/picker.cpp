#include "core.h"

Picker::Picker(vec4 rect) : rect(rect) {
    God::context.render_target.read_framebuffer(layer_data, rect, 1);
    God::context.render_target.read_framebuffer(primitive_data, rect, 2);
}

Picker::Picker() : Picker(vec4{0, 0, 
                        static_cast<double>(God::context.render_target.width), 
                        static_cast<double>(God::context.render_target.height)}) {
}