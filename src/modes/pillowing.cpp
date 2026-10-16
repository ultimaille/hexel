#include "pillowing.h"
using namespace UM::Linear;




Pillow::Pillow(Hexahedra& hex, CellFacetAttribute<int>& pillow) : hex(hex), pillow(pillow){}


int  Pillow::map_corners_to_output_vertices(CellCornerAttribute<int>& c2ov){
    // STEP 1: do the job, but output vertices id are mixed
    // define each output vertex as a set of corners issued from an input vertex
    // => represented by c2ov and ov2iv
    // ov/iv are for output vertex / input vertex
    int nov = -1;
    std::vector<int> ov2iv;
    {
        DisjointSet ds(hex.ncorners());
        for (auto h : hex.iter_halfedges()){
            if (pillow[h.facet()] == 1) continue;
            auto opp = h.opposite_c();
            if (!opp.active()) continue;
            if (pillow[opp.facet()] == 1) continue;
            ds.merge(h.from_corner(), opp.to_corner());
        }
        nov = ds.get_sets_id(c2ov.ptr->data);

        ov2iv.resize(nov, -1);
        for (auto c : hex.iter_corners()) ov2iv[c2ov[c]] = c.vertex();
    }


    // STEP 2: reorder output vertices to preverve input vertices ids

    // define the output vertex that will have the index its source input vertex
    std::vector<bool> ov_is_resp(nov, true);
    {
        // do the job
        for (auto h : hex.iter_halfedges())
            if (pillow[h.facet()] == 1)
                ov_is_resp[c2ov[h.from_corner()]] = false;

        // each input vertex is referenced by exaclty one responsible output vertex
        if (![&](){
            PointAttribute<int> nresp(hex, 0);
            FOR(ov, nov) if (ov_is_resp[ov]) nresp[ov2iv[ov]]++;
            FOR(v, hex.nverts()) if (nresp[v] != 1) return false;
            return true;
            }()) Log::abort("pad flags are not ok");
    }

    //sort c2ov
    {
        std::vector<bool> used(nov, false);
        std::vector<int> reorder(nov, -1);

        FOR(ov, nov){
            if (!ov_is_resp[ov]) continue;
            int iv = ov2iv[ov];
            um_assert(!used[iv]);
            reorder[ov] = iv;
            used[iv] = true;
        }

        std::vector<int> free_id;
        std::vector<int> free_val;
        FOR(ov, nov) {
            if (reorder[ov] == -1)	free_id.push_back(ov);
            if (!used[ov])			free_val.push_back(ov);
        }
        FOR(k, free_id.size()) reorder[free_id[k]] = free_val[k];


        // apply the reordering
        CellCornerAttribute<int> tmp(hex);
        for (auto c : hex.iter_corners())  tmp[c] = c2ov[c];
        for (auto c : hex.iter_corners()) c2ov[c] = reorder[tmp[c]];
    }
    return nov;


}



std::vector<std::pair<int, int> > Pillow::get_output_vertices_around_edge(CellCornerAttribute<int>& c2ov, EdgeGraph& eg, EdgeGraph::Edge e){
    Volume::Halfedge h = eg.halfedge_from_edge(e);
    std::vector<std::pair<int, int> > ov_ids;
    auto cir = h;
    do {
        auto opp = cir.opposite_c();
        if (!opp.active()) {
            cir = -1;
            break;
        }
        if (pillow[cir.facet()] || pillow[opp.facet()])
            ov_ids.push_back({ c2ov[cir.from_corner()],c2ov[cir.to_corner()] });
        cir = opp.opposite_f();
    } while (cir != -1 && cir != h);
    if (cir == -1) return std::vector<std::pair<int, int> >();//continue;// do not generate hex on boundary edges --- should be taken into account by pre-padding
    return ov_ids;
}



void Pillow::pillow_without_boundary_faces(double thickness){

    CellCornerAttribute<int> c2ov(hex);

    int nov = map_corners_to_output_vertices(c2ov);
    int niv = hex.nverts();
    int nih = hex.ncells();

    // create output vertices
    {
        hex.points.create_points(nov - niv);
        PointAttribute<vec3> pos(hex); FOR(v, hex.nverts()) pos[v] = hex.points[v];
        for (auto c : hex.iter_corners()) hex.points[c2ov[c]] = pos[c.vertex()];
    }


    //---------------------------------------------------------------------------------------------------
    // at this point we should have all vertices ready, so it's time to update the combinatorial structure


    // compute new hexes to add, using the input connectivity 
    std::vector<int> new_hexes;
    {
        // let's define an usefull fonction for pillow intersections
        EdgeGraph eg(hex);

        //let's start with the easiest hex: those produced by facets
        for (auto f : hex.iter_facets()) if (pillow[f]){
            Volume::Halfedge h = f.halfedge(0);
            Volume::Halfedge opp = h.opposite_c();
            int verts[8] = {
                h.from_corner(),h.to_corner(),h.prev().from_corner(),h.next().to_corner(),
                opp.to_corner(),opp.from_corner(),opp.next().to_corner(),opp.prev().from_corner()
            };
            FOR(lv, 8) new_hexes.push_back(c2ov[verts[lv]]);
        }



        // now add hexes for edges

        for (auto e : eg.iter_edges()){
            um_assert(e.opposite().active());
            if (e < e.opposite()) continue;

            std::vector<std::pair<int, int> > ov_ids = get_output_vertices_around_edge(c2ov, eg, e);



            if (ov_ids.size() == 4){
                int verts[8] = {
                    ov_ids[0].first,ov_ids[1].first,ov_ids[3].first,ov_ids[2].first,
                    ov_ids[0].second,ov_ids[1].second,ov_ids[3].second,ov_ids[2].second
                };
                FOR(lv, 8) new_hexes.push_back(verts[lv]);
            }
        }

        // and finally, add hex to vertices 

        PointAttribute<bool> on_border(eg, false);
        for (auto f : hex.iter_facets()) if (!f.opposite().active())for (auto cir : f.iter_halfedges()) on_border[cir.from()] = true;

        for (auto v : eg.iter_vertices()){
            if (on_border[v]) continue;
            std::vector<std::vector<int>> quads;
            for (auto e : v.iter_edges()){
                um_assert(e.opposite().active());
                std::vector<std::pair<int, int> > ov_ids = get_output_vertices_around_edge(c2ov, eg, e);
                if (ov_ids.size() != 4) continue;
                quads.push_back({ ov_ids[0].first,ov_ids[1].first,ov_ids[2].first,ov_ids[3].first });
            }

            if (quads.size() != 6) continue;

            int new_hex[8] = { -1,-1,-1,-1,quads[0][0],quads[0][1],quads[0][3],quads[0][2] };

            for (auto quad : quads)FOR(lv0, 4){
                int lv1 = (lv0 + 1) % 4;
                int lv2 = (lv1 + 1) % 4;
                int lv3 = (lv2 + 1) % 4;
                if (quad[lv0] == new_hex[5] && quad[lv1] == new_hex[4]){
                    new_hex[0] = quad[lv2];
                    new_hex[1] = quad[lv3];
                }
                if (quad[lv0] == new_hex[6] && quad[lv1] == new_hex[7]){
                    new_hex[2] = quad[lv3];
                    new_hex[3] = quad[lv2];
                }
            }

            FOR(lv, 8) new_hexes.push_back(new_hex[lv]);
        }
    }



    // remap the input hexes to the output vertices
    {
        hex.disconnect();
        FOR(c, hex.ncells()) FOR(lv, 8) hex.vert(c, lv) = c2ov[8 * c + lv];

    }

    // add the new hexes to the mesh
    {
        int off_c = hex.create_cells(new_hexes.size() / 8);
        FOR(c, new_hexes.size() / 8) FOR(lv, 8)
            hex.vert(off_c + c, lv) = new_hexes[8 * c + lv];

    }

    hex.connect();


    // smooth new vertices position
    {
        EdgeGraph eg(hex);
        PointAttribute<bool> lock(hex);
        FOR(v, hex.nverts()) lock[v] = (v < niv);
        FOR(iteration, 3){
            FOR(dim, 3){
                LeastSquares ls(hex.nverts());
                FOR(v, hex.nverts()) if (lock[v]) ls.fix(v, hex.points[v][dim]);
                for (auto e : eg.iter_edges()) if (lock[e.from()]){
                    double c = thickness;
                    auto h = eg.halfedge_from_edge(e);
                    for (auto cir : h.iter_CCW_around_edge())
                        if (cir.cell() < nih) c = 1. - thickness;
                    ls.add_to_energy(std::sqrt(c) * (X(e.from()) - X(e.to())));
                }
                ls.solve();
                FOR(v, hex.nverts()) hex.points[v][dim] = ls.value(v);
            }

            PointAttribute<int> n1ringlock(hex, 0);
            for (auto e : eg.iter_edges()) if (lock[e.from()]) n1ringlock[e.to()]++;
            FOR(v, hex.nverts()) lock[v] = lock[v] || (n1ringlock[v] > 1);
        }
    }
}


void Pillow::pad()		{
    PointAttribute<int> out_vert(hex, -1);
    int nverts = hex.nverts();
    //create one output vertex for each boundary vertex
    for (auto f : hex.iter_facets()) if (!f.opposite().active()) for (auto h : f.iter_halfedges()) if (out_vert[h.from()] == -1) {
        out_vert[h.from()] = nverts;
        nverts++;
    }

    std::vector<int> new_hexes;
    // generate new hex array
    for (auto f : hex.iter_facets()) if (!f.opposite().active()){
        int verts[8] = {
            f.vertex(0),f.vertex(1),f.vertex(3),f.vertex(2),
            -1,-1,-1,-1
        };
        FOR(lv, 4) verts[4 + lv] = out_vert[verts[lv]];
        FOR(lv, 8) new_hexes.push_back(verts[lv]);
    }


    // add new hexes to the mesh and assign geometry to new vertices
    {

        hex.points.create_points(nverts - hex.nverts());
        //set new point geometry, for debug purpose only
        {
            PointAttribute<vec3> decal(hex, vec3(0, 0, 0));
            for (auto f : hex.iter_facets()) if (!f.opposite().active()) for (auto h : f.iter_halfedges()) decal[h.from()] -= .05 * Quad3(f).normal();
            for (auto v : hex.iter_vertices()) if (out_vert[v] != -1) hex.points[out_vert[v]] = v.pos() + decal[v];
        }

        int off_c = hex.create_cells(new_hexes.size() / 8);
        FOR(c, new_hexes.size() / 8) FOR(lv, 8)
            hex.vert(off_c + c, lv) = new_hexes[8 * c + lv];

    }
    hex.connect();
}

void Pillow::unpad(){
    std::vector<bool> c2kill(hex.ncells(), false);

    for (auto f : hex.iter_facets())
        if (!f.opposite().active())
            c2kill[f.cell()] = true;
    hex.disconnect();
    hex.delete_cells(c2kill);
    hex.delete_isolated_vertices();
    hex.connect();
}

void Pillow::pillow_with_boundary_faces(double thickness){
    um_assert(pillow.default_value == 0);
    pad();

    {// update the set of faces to pillow 
        EdgeGraph eg(hex);
        EdgeAttribute<int> edge_is_on_border(eg, 0);
        for (auto h : hex.iter_halfedges()) if (!h.facet().opposite().active()) edge_is_on_border[eg.edge_from_halfedge(h)] = 1;

        for (auto h : hex.iter_halfedges()) if (h.facet().opposite().active() && edge_is_on_border[eg.edge_from_halfedge(h)]){
            auto e = eg.edge_from_halfedge(h.next().next());

            int count = 0;
            auto cir = h.next().next();
            do {
                auto opp = cir.opposite_c();
                um_assert(opp.active());
                if (pillow[cir.facet()]) count++;
                if (pillow[opp.facet()]) count--;
                cir = opp.opposite_f();
            } while (cir != h.next().next());
            if (count == -1) pillow[h.facet()] = 1;
        }
    }
    //Drop(hex, pillow)._skip_value(0).apply("padded pillow faces");
    //DropVolume(hex).apply("padded hex");
    pillow_without_boundary_faces(thickness);
    unpad();
}

void Pillow::apply(double thickness ){
    um_assert(thickness > 0 && thickness < 1);
    for (auto f : hex.iter_facets()) if (pillow[f] == 1) {
        pillow_with_boundary_faces(thickness);
        return;
    }
    pillow_without_boundary_faces(thickness);
}

