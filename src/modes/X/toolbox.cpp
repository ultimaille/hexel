#include <modes/X/toolbox.h>
#include <core/basic.h>
#include <ultimaille/all.h>


ToolBoxHexahedra::ToolBoxHexahedra(Hexahedra& hex) :hex(hex){}
    void ToolBoxHexahedra::merge_vertices(int source, int target){
        FOR(c, hex.ncells())FOR(lv, 8) if (hex.vert(c, lv) == source)hex.vert(c, lv) = target;
    }
    bool ToolBoxHexahedra::validate_topo(std::vector<int>& conficts){
        {// each hexahedron is valid
            // vertices in good range
            for (auto c : hex.iter_corners()) if (c.vertex() < 0 || c.vertex() >= hex.nverts()) return false;

            // no duplicated vertices
            FOR(c, hex.ncells())FOR(lv0, 8) FOR(lv1, lv0) if (hex.vert(c, lv0) == hex.vert(c, lv1)) { conficts.push_back(hex.vert(c, lv0)); };
        }

        // compute some connectivity
        PointAttribute<int> v2c(hex, -1);
        CellCornerAttribute<int> c2c(hex, -1);
        for (auto c : hex.iter_corners()) {
            c2c[c] = v2c[c.vertex()];
            v2c[c.vertex()] = c;
        }

        {
            for (auto c : hex.iter_corners()) {
                Volume::Corner cir(hex, v2c[c.vertex()]);
                while (cir.active()){
                    if (c == cir) {
                        cir = c2c[cir];
                        continue;
                    }
                    int nb_shared_vertices = 0;
                    FOR(lv_c, 8)FOR(lv_cir, 8)
                        if (hex.vert(c.cell(), lv_c) == hex.vert(cir.cell(), lv_cir))
                            nb_shared_vertices++;

                    //plop(nb_shared_vertices);
                    switch (nb_shared_vertices){
                    case 0: Log::error("Cannot not share vertices");
                    case 1:                     cir = c2c[cir];
                        continue;// share a vertex
                    case 2:{
                        // share and edge ?
                        bool is_an_edge = false;
                        Volume::Halfedge h_c = c.halfedge();
                        FOR(i, 3){
                            Volume::Halfedge h_cir = c.halfedge();
                            FOR(j, 3){
                                is_an_edge = is_an_edge || (h_c.to() == h_cir.to());
                                h_cir = h_cir.prev().opposite_f();
                            }
                            h_c = h_c.prev().opposite_f();
                        }
                        if (!is_an_edge) {
                            FOR(lv, 8) conficts.push_back(c.cell().vertex(lv));
                            FOR(lv, 8) conficts.push_back(cir.cell().vertex(lv));
                        }//return false;
                        cir = c2c[cir];
                        continue;
                    }
                    case 4:{
                        // share a facet?
                        bool is_a_facet = false;
                        Volume::Halfedge h_c = c.halfedge();
                        FOR(i, 3){
                            Volume::Halfedge h_cir = cir.halfedge();
                            FOR(j, 3){
                                is_a_facet = is_a_facet || (
                                    h_c.to() == h_cir.prev().from()
                                    && h_c.next().to() == h_cir.prev().prev().from()
                                    && h_c.next().next().to() == h_cir.prev().prev().prev().from()
                                    );
                                h_cir = h_cir.prev().opposite_f();
                            }
                            h_c = h_c.prev().opposite_f();
                        }
                        if (!is_a_facet) {
                            FOR(lv, 8) conficts.push_back(c.cell().vertex(lv));
                            FOR(lv, 8) conficts.push_back(cir.cell().vertex(lv));
                        }//return false;
                        cir = c2c[cir];
                        continue;
                    }
                    default:
                        FOR(lv, 8) conficts.push_back(c.cell().vertex(lv));
                        FOR(lv, 8) conficts.push_back(cir.cell().vertex(lv));
                    }
                    cir = c2c[cir];
                }
            }
        }
        return conficts.empty();
    }
    bool ToolBoxHexahedra::validate_geom(std::vector<int>& conficts) {
        for (auto c : hex.iter_corners()){
            Volume::Halfedge h = c.halfedge();
            //plop(int(h));
            Tetrahedron tet(h.from().pos(), h.to().pos(), h.opposite_f().next().to(), h.prev().from().pos());
            //plop(tet.volume());
            if (tet.volume() < 0) FOR(lv, 8) conficts.push_back(c.cell().vertex(lv));
        }
        return conficts.empty();
    }



    ToolBoxTriangles::ToolBoxTriangles(Triangles& tri) :tri(tri){}
    vec3 ToolBoxTriangles::next_intersection(vec3 O, vec3 dir){
        static const double infty = 1e20;
        double best_t = infty;
        for (auto f : tri.iter_facets()){
            Triangle3 tr(f);
            vec3 A = tr[0]; vec3 B = tr[1]; vec3 C = tr[2];
            double t = 1e20;


            const double EPSILON = 1e-9;
            vec3 AB = B - A;
            vec3 AC = C - A;
            vec3 h = cross(dir, AC);
            double det = AB * h;
            if (std::abs(det) < EPSILON) continue;
            double invDet = 1.0 / det;
            vec3 s = O - A;
            double u = invDet * (s * h);
            if (u < 0.0 || u > 1.0) continue;
            vec3 q = cross(s, AB);
            double v = invDet * (dir * q);
            if (v < 0.0 || u + v > 1.0) continue;
            t = invDet * (AC * q);
            if (t < EPSILON) continue;
            best_t = std::min(t, best_t);
        }
        if (best_t < infty)
            return O + best_t * dir;
        return O;
    }


