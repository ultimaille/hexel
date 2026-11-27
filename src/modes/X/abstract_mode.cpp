#include <modes/X/abstract_mode.h>
#include <modes/X/root_mode.h>


namespace ModeX{
    ModeX::AbstractMode::AbstractMode(RootMode& mode) :XCFview(mode), interactor_layer_selection(*this){
        interactors.push_back(&interactor_camera);
        interactors.push_back(&interactor_layer_selection);
    }

    void AbstractMode::handle_camera_and_layerselect(Event event){
       interactor_camera.handle(event);
       interactor_layer_selection.handle(event);
    }
    void AbstractMode::handle(Event event) {
        if (event.who != root.mouse) {
            mode.picker.reset();
            //need_update_picker = true;
        }
    };

    void AbstractMode::update() {
        //if (need_update_picker){
        //    mode.picker.reset();
        //    need_update_picker = false;
        //}
    }
};