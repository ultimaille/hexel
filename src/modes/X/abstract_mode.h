#pragma once
#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/X/generic_mouse_reacts.h>
#include <modes/X/scene.h>
#include <modes/X/inspect.h>
#include <core/mode.h>

namespace ModeX{

    struct RootMode;





    struct AbstractMode : public Mode, public XCFview{
        AbstractMode(RootMode& mode);

        // manage camera + layer selection
        InteractorTrackBallCamera interactor_camera;
        InteractorLayerSelection interactor_layer_selection;
        void handle_camera_and_layerselect(Event event);


        Scene scene(){ return Scene(mode); }
        Inspect inspect(){ return Inspect(mode); }

        virtual void define_gui(){};
        virtual void handle(Event event) ;
        virtual void update();
        //bool need_update_picker = false;
    };




}

