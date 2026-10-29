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

    // access to xcf bjects
    ObjectId hex_id();
    ObjectId pl_id();
    ObjectId pts_id();
    ObjectId tri_id();
    ObjectId feature_edge_id();
    ObjectId feature_vertex_id();


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
    void load_project();
    void import_new_project_from_CAD(std::string filename);

    // manage submodules
    struct SubMode : public Mode{
        SubMode(HexModelerMode& mode) :mode(mode){}
        inline ObjectId hex_id()                  { return mode.hex_id(); }
        inline ObjectId pl_id()                   { return mode.pl_id(); }
        inline ObjectId pts_id()                  { return mode.pts_id(); }
        inline ObjectId tri_id()                  { return mode.tri_id(); }
        inline ObjectId feature_edge_id()         { return mode.feature_edge_id(); }
        inline ObjectId feature_vertex_id()       { return mode.feature_vertex_id(); }

        Volume::Halfedge hex_halfedge_from_edge(int e){ return mode.hex_halfedge_from_edge(e); }
        void update_hex(){ mode.update_hex(); }
        virtual void define_gui(){};
        virtual void handle(Event event) {};
        HexModelerMode& mode;
    };
    Registry<SubMode> modes;
    int active_mode_id;
    void set_active_mode(std::string s);
    void define_gui();
    void handle(Event event);
};


