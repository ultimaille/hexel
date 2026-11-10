#pragma once
#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/generic_mouse_reacts.h>
#include <core/mode.h>





struct HexModelerMode : public Mode{

    HexModelerMode();

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
    std::string working_directory;
    std::string project_directory;
    std::string CAD_file;
    void reset_xcf();
    void load_project(std::string directory);
    void save_project(std::string directory);
    void import_new_project_from_CAD(std::string filename);



    bool xcf_is_valid_for_editing();
    void reset_layers_options();

    // manage undo
    int undo_begin = -1;
    int undo_stack_size = 5;
    int undo_id=-1;
    void backup();
    void undo();

    // manage submodules
    struct SubMode : public Mode{
        SubMode(HexModelerMode& mode) :mode(mode){}
        inline MeshPath hex_id()                  { return mode.hex_id(); }
        inline MeshPath pl_id()                   { return mode.pl_id(); }
        inline PointSetPath pts_id()                  { return mode.pts_id(); }
        inline MeshPath geom_tri_id()             { return mode.geom_tri_id(); }
        inline MeshPath geom_edge_id()            { return mode.geom_edge_id(); }
        inline PointSetPath geom_pts_id()             { return mode.geom_pts_id(); }

        Volume::Halfedge hex_halfedge_from_edge(int e){ return mode.hex_halfedge_from_edge(e); }
        void update_hex(){ mode.update_hex(); }
        virtual void define_gui(){};
        virtual void handle(Event event) {};
        HexModelerMode& mode;
    };
    Registry<SubMode> modes;
    int active_mode_id;
    void set_active_mode(std::string s);

    void header_gui_for_submodes();
    void define_gui();
    void handle(Event event);
    void reset_gui();
};


