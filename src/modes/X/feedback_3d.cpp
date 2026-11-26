#include <modes/X/feedback_3d.h>
#include <layers/pointset_layer.h>
#include <layers/polyline_layer.h>
#include <layers/triangle_layer.h>


MultiMeshPath::PointSet FeedBack3D::pts_id(){ return root.xcf[name].pointset; }
MultiMeshPath::PolyLines::PolyLine FeedBack3D::pl_id(){ return root.xcf[name].polylines[""]; }
MultiMeshPath::Surfaces::Surface FeedBack3D::tri_id(){ return root.xcf[name].triangles[""]; }

void FeedBack3D::activate(){
    God::xcf.add(name);
    int group = God::layers.add_group(name);
    MultiMesh& mm = root.xcf[name];
    mm.polylines.try_emplace("");
    mm.polylines[""].mesh.points = mm.pointset;
    mm.triangles.try_emplace("");
    mm.triangles[""].mesh.points = mm.pointset;

    {// point set
        PointSetLayer& layer = God::layers.add<PointSetLayer>(name + "Pts", group);
        layer.init(pts_id());
        layer.set_default_values();
        layer.point_renderer.radius_in_pixel = 5;
        FOR(d,3) layer.point_renderer.color[d] = d==2?255:0;
    }
    {
        PolyLineLayer& layer = God::layers.add<PolyLineLayer>(name + "Edges", group);
        layer.init(pl_id());
        layer.set_default_values();
    }
    {
        TrianglesLayer& layer = God::layers.add<TrianglesLayer>(name + "Tri", group);
        layer.init(tri_id());
        layer.set_default_values();
    }
    God::layers.find_group_ptr(name)->blend_op = Group::BlendOp::AVERAGE;;
    //for (auto& group : God::layers.groups) if (group.name == name)
    //        group.blend_op = Group::BlendOp::AVERAGE;

}
void FeedBack3D::desactivate(){
    if (activated()) God::xcf.kill_multimesh(name);
}
bool FeedBack3D::activated(){
    return God::xcf.contains(name);
}


void FeedBack3D::begin_mesh_update(){
    um_assert(activated());
    MultiMesh& mm = root.xcf[name];
    std::vector<bool> edges_to_kill(mm.polylines[""].mesh.nedges(), true);
    mm.polylines[""].mesh.disconnect();
    mm.polylines[""].mesh.delete_edges(edges_to_kill);
    std::vector<bool> triangles_to_kill(mm.triangles[""].mesh.nfacets(), true);
    mm.triangles[""].mesh.disconnect();
    mm.triangles[""].mesh.delete_facets(triangles_to_kill);
    std::vector<bool> points_to_kill(mm.pointset.size(), true);
    mm.pointset.delete_points(points_to_kill);
}

void FeedBack3D::add_polyline(std::vector<vec3>& verts){
    um_assert(activated());
    MultiMeshPath mm_id = root.xcf[name];
    if (!mm_id.valid()) activate();
    MultiMesh& mm = God::xcf[name];
    int offset_v = mm.pointset.create_points(verts.size());
    FOR(lv, verts.size()) mm.pointset[offset_v + lv] = verts[lv];
    if (verts.size() > 1){
        PolyLine& pl = mm.polylines[""].mesh;
        pl.disconnect();
        int offset_e = pl.create_edges(verts.size() - 1);
        FOR(le, verts.size() - 1) FOR(lv, 2) pl.vert(offset_e + le, lv) = offset_v + le + lv;
        pl.connect();
    }
}

void FeedBack3D::add_points(std::vector<vec3>& verts){
    um_assert(activated());
    MultiMeshPath mm_id = root.xcf[name];
    if (!mm_id.valid()) activate();
    MultiMesh& mm = God::xcf[name];
    int offset_v = mm.pointset.create_points(verts.size());
    FOR(lv, verts.size()) mm.pointset[offset_v + lv] = verts[lv];
}

void FeedBack3D::end_mesh_update(){
    um_assert(activated());
    root.xcf[name].pointset.broadcast(Event::UPDATED);
    root.xcf[name].polylines[""].broadcast(Event::UPDATED);
    root.xcf[name].triangles[""].broadcast(Event::UPDATED);
}
