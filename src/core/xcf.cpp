#include <ultimaille/all.h>
#include "xcf.h"
#include "core.h"

std::string MultiMesh::collection_names[8] = {
"polylines",
"triangles","quads","polygons",
"tetrahedra","hexahedra","wedges","pyramids"
};



void XCF::kill_mesh(ObjectId obj) {
    um_assert(obj.chunks[0] == chunk_xcf);
    um_assert(obj.chunks.size()==4);
    MultiMesh& mm = (*this)[obj.chunks[1]];
    std::string mesh_type = obj.chunks[2];
    std::string mesh = obj.chunks[3];
    if (mesh_type == chunk_polylines)   mm.polylines.erase(mesh);
    if (mesh_type == chunk_triangles)   mm.triangles.erase(mesh);
    if (mesh_type == chunk_quads)       mm.quads.erase(mesh);
    if (mesh_type == chunk_polygons)    mm.polygons.erase(mesh);
    
    if (mesh_type == chunk_tetrahedra)  mm.tetrahedra.erase(mesh);
    if (mesh_type == chunk_hexahedra)   mm.hexahedra.erase(mesh);
    obj.emit(KILLED);
}

void XCF::kill_multimesh(const std::string & mm_name) {
    MultiMesh& mm = (*this)[mm_name];
    std::vector<std::string> to_kill;
    for (auto& [name, obj] : mm.polylines)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(ObjectId({ chunk_xcf,mm_name,chunk_polylines, name }));

    for (auto& [name, obj] : mm.triangles)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(ObjectId({ chunk_xcf,mm_name,chunk_triangles, name }));


	std::map<std::string, MultiMesh>::erase(mm_name);
	ObjectId({ chunk_xcf ,  mm_name }).emit(KILLED);
    ObjectId({ chunk_xcf, mm_name ,chunk_pointset }).emit(KILLED);
}
MultiMesh& XCF::add(std::string str) {
    ObjectId({ chunk_xcf,str }).emit(CREATED);
    return std::map<std::string, MultiMesh>::operator[](str);
}

MultiMesh::MeshAttr<Triangles, SurfaceAttributes>& XCF::add_triangles(std::string mm_name, std::string tri_name) {
    auto& collection = (*this)[mm_name].triangles;
    collection.try_emplace(tri_name);
    plop(collection.contains(tri_name));
    plop(tri_name);
    ObjectId obj({ chunk_xcf,mm_name,chunk_triangles,tri_name });
    obj.show();
    obj.emit(CREATED);
    return collection[tri_name];
}
