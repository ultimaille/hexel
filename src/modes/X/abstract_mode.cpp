#include <modes/X/abstract_mode.h>


namespace ModeX{
    ModeX::AbstractMode::AbstractMode(RootMode& mode) :XCFview(mode), interactor_layer_selection(*this){
        interactors.push_back(&interactor_camera);
        interactors.push_back(&interactor_layer_selection);
    }

    void AbstractMode::handle_camera_and_layerselect(Event event){
       interactor_camera.handle(event);
       interactor_layer_selection.handle(event);
    }
    void AbstractMode::update() {}
};