#pragma once
#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/generic_mouse_reacts.h>
#include <core/mode.h>

//struct HexModelerData{
//    HexModelerData();
//    std::string working_directory;
//    std::string project_directory;
//    std::string CAD_file;
//    int undo_begin = -1;
//    int undo_stack_size = 5;
//    int undo_id = -1;
//    int active_mode_id=0;
//};
struct HexModelerMode;

struct HexModelerSubMode : public Mode{
    HexModelerSubMode(HexModelerMode& mode);
    HexModelerMode& mode;

    // access to xcf objects
    static MeshPath hex_id();
    static MeshPath pl_id();
    static PointSetPath pts_id();
    static MeshPath geom_tri_id();
    static MeshPath geom_edge_id();
    static PointSetPath geom_pts_id();

    // update coherence constraints
    Volume::Halfedge hex_halfedge_from_edge(int e);
    void update_hex();

    // manage camera + layer selection
    MouseReactTrackBallCamera camera_react;
    MouseReactLayerSelection layer_selection_react;
    void handle_camera_and_layerselect(Event event);

    // manage import/load/write of project
    void reset_xcf();
    void load_project(std::string directory);
    void save_project(std::string directory);
    
    // manage undo
    void backup();
    void undo();

    bool xcf_is_valid_for_editing();
    void reset_layers_options();


    virtual void define_gui(){};
    virtual void handle(Event event) {};

};





