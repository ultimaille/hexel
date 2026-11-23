#pragma once
#include "misc/purgatory.h"
#include <layers/ssao.h>
#include <layers/basic_layers.h>
#include "algo.h"
#include <algo/pillowing.h>
#include <algo/raster_iterator.h>
#include <modes/generic_mouse_reacts.h>
#include <core/mode.h>

struct ModeX;


struct ModeXObject{
    // access to xcf objects
    static MeshPath hex_id();
    static MeshPath pl_id();
    static PointSetPath pts_id();
    static MeshPath geom_tri_id();
    static MeshPath geom_edge_id();
    static PointSetPath geom_pts_id();
};


struct ModeXScene : public ModeXObject{
    ModeXScene(ModeX& mode) : mode(mode){}
    ModeX& mode;

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

struct ModeXInspect : public ModeXObject{
    ModeXInspect(ModeX& mode) : mode(mode){}
    ModeX& mode;

    struct Embedding{int dim = -1;int id = -1;};
    inline bool on_point(vec3 P, vec3 point){ return (P - point).norm2() < 1e-20; }
    inline bool on_edge(vec3 P, Segment3 s){ return (P - s.nearest_point(P)).norm2() < 1e-15; }
    inline bool on_triangle(vec3 P, Triangle3 tr){ return (P - tr.nearest_point(P)).norm2() < 1e-15; }

    inline Embedding get_embedding(vec3 P) {
        Triangles& tri = geom_tri_id();
        PolyLine& pl = geom_edge_id();
        for (auto v : tri.iter_vertices()) if (on_point(v.pos(), P)) return { 0,v };
        for (auto e : pl.iter_edges())if (on_edge(P, e)) return { 1,e };
        for (auto f : tri.iter_facets())if (on_triangle(P, f)) return { 2,f };
        return { -1,-1 };
    }
};


struct SubModeX : public Mode, public ModeXObject{
    SubModeX(ModeX& mode);
    ModeX& mode;

    // manage camera + layer selection
    InteractorTrackBallCamera camera_react;
    InteractorLayerSelection layer_selection_react;
    void handle_camera_and_layerselect(Event event);


    ModeXScene scene(){ return ModeXScene(mode); }
    ModeXInspect inspect(){ return ModeXInspect(mode); }

    virtual void define_gui(){};
    virtual void handle(Event event) {};
    virtual void update() ;

};





