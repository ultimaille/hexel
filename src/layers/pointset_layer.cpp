#include "pointset_layer.h"
#include "core/layers.h"
#include "core/camera.h"
#include "core/picker.h"
#include "core/xcf.h"
#include "core/render_target.h"
#include "core/window.h"
#include "core/panels.h"
#include "core/shaders.h"
#include "core/mode.h"

#include <misc/purgatory.h>
#include <format>

PointSetLayer::PointSetLayer() { primitive_renderer.layer_id = id; }

void PointSetLayer::generate_gui(std::string name){
    primitive_renderer.generate_gui(name);
    Layer::generate_gui(name);
}

int PointSetLayer::primitive_type(){ return 0; }


void PointSetLayer::reset(){
    PointSet& ps = mesh;
    PointSetAttributes& attr = mesh;

    PointAttribute<float> value(ps);
    FOR(v, ps.size()){
        value[v] = ps[v][0];
    }

    PointAttribute<bool> visible("visible", attr, ps, true);
    PointAttribute<int> primitive_id(ps);
    FOR(v, ps.size())
        primitive_id[v] = v;

    primitive_renderer.init();
    primitive_renderer.update(ps, visible, value, primitive_id);
}
void PointSetLayer::init(ObjectId obj){
    mesh = obj;
    reset();
}

void PointSetLayer::render(){
    if (visible)primitive_renderer.render();
}

void PointSetLayer::destroy() {
    primitive_renderer.destroy();
}