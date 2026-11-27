#pragma once
#include "misc/purgatory.h"
//#include <layers/ssao.h>
//#include <layers/basic_layers.h>
#include "algo.h"
//#include <algo/pillowing.h>
//#include <algo/raster_iterator.h>
//#include <modes/X/generic_mouse_reacts.h>
//#include <modes/X/inspect.h>
#include <core/mode.h>

namespace ModeX{

    struct RootMode;

    struct XCFview{
        XCFview(RootMode& mode) :mode(mode){}
        // access to xcf objects
        static MeshPath hex_id();
        static MeshPath pl_id();
        static PointSetPath pts_id();
        static MeshPath geom_tri_id();
        static MeshPath geom_edge_id();
        static PointSetPath geom_pts_id();
        Picker& picker();
        RootMode& mode;
    };


    struct Scene : public XCFview{
        Scene(RootMode& mode) : XCFview(mode){}

        // manage import/load/write of project
        void reset_xcf();
        void load_project(std::string directory);
        void save_project(std::string directory);
        bool xcf_is_valid_for_editing();

        // manage undo
        void backup();
        void forget_last_backup();
        void undo();

        void restore_default_scene_rendering();

        // update coherence constraints
        bool update_hex();
    };
}

