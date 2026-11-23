#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/X/root_mode.h>
#include <modes/X/mode_start.h>
#include <modes/X/mode_layer_split.h>
#include <modes/X/mode_feature_edit.h>
#include <modes/X/mode_basic_edit.h>
#include <modes/X/scene.h>
#include <modes/X/inspect.h>



//#define WORKING_DIR std::string("/home/ssloy/+/hexel/build/NICO/")
#define WORKING_DIR std::string("C:\\NICOTMP\\tmp\\")

namespace ModeX{


    ModeX::AbstractMode::AbstractMode(RootMode& mode) :XCFview(mode),layer_selection_react(*this){}




    // manage camera + layer selection
    void AbstractMode::handle_camera_and_layerselect(Event event){
        camera_react.handle(event);
        layer_selection_react.handle(event);
    }







    void AbstractMode::update() {}

};