#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/X/root_mode.h>
#include <modes/X/mode_start.h>
#include <modes/X/mode_layer_split.h>
#include <modes/X/mode_feature.h>
#include <modes/X/mode_basic.h>
#include <modes/X/scene.h>
#include <modes/X/inspect.h>



//#define WORKING_DIR std::string("/home/ssloy/+/hexel/build/NICO/")
#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")

namespace ModeX{
    ModeX::AbstractMode::AbstractMode(RootMode& mode) :XCFview(mode), interactor_layer_selection(*this){
        interactors.push_back(&interactor_camera);
        interactors.push_back(&interactor_layer_selection);
    }

    // manage camera + layer selection
    void AbstractMode::handle_camera_and_layerselect(Event event){
       interactor_camera.handle(event);
       interactor_layer_selection.handle(event);
    }
    void AbstractMode::update() {}
};