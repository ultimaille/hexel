#pragma once
#include <modes/X/feedback_3d.h>
#include <modes/X/abstract_mode.h>

namespace ModeX{

    ENUM(QuadToHexGeomStrategy, OFFSET_PROP_TO_QUAD, RAY_TRACE, FRAME_FIELD);
    ENUM(QuadToHexTopoStrategy, AUTO_MERGE, NO_MERGE);


    // algorithms that are ready to call from UI
    struct Commands {

        Commands(AbstractMode& mode, bool backup = true);
        static FeedBack3D feedback;

        ~Commands();
        AbstractMode& mode;

        void update_panic_in_xcf(std::vector<int>& conflicts_topo, std::vector<int>& conflicts_geom);


        void merge_vertices(int source, int target);
        void move_vertex(int moving_vertex, vec3 new_pos);

        //enum Quad2HexGeomStrategy { OFFSET_BY_QUAD_NORMAL, RAYTRACE };
        enum Quad2HexTopoStrategy { AUTO_MERGE, NO_MERGE };
        void create_hex_from_quad(std::vector<vec3>& pts, QuadToHexGeomStrategy geom_strat, QuadToHexTopoStrategy topo_strat);
        void extend_mesh_from_face(int f_id);

        void kill_hexes(std::vector<bool>& to_kill);

    };
};
