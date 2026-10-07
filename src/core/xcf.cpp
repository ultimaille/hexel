#include <ultimaille/all.h>
#include "xcf.h"
#include "core.h"
#include "camera.h"
#include "picker.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"
#include "mode.h"

std::string MultiMesh::collection_names[8] = {
"polylines",
"triangles","quads","polygons",
"tetrahedra","hexahedra","wedges","pyramids"
};

using namespace events;

void XCF::kill_mesh(ObjectId obj) {
    um_assert(obj.path == POLYLINES || obj.path == TRIANGLES || obj.path == QUADS || obj.path == POLYGONS || obj.path == TETRAHEDRA || obj.path == HEXAHEDRA || obj.path == WEDGES || obj.path == PYRAMIDS);
    um_assert(obj.names.size()==2);
    MultiMesh& mm = (*this)[obj.names[0]];
    std::string mesh = obj.names[1];

    switch (obj.path) {
        case POLYLINES:  mm.polylines.erase(mesh);  break;
        case TRIANGLES:  mm.triangles.erase(mesh);  break;
        case QUADS:      mm.quads.erase(mesh);      break;
        case POLYGONS:   mm.polygons.erase(mesh);   break;
        case TETRAHEDRA: mm.tetrahedra.erase(mesh); break;
        case HEXAHEDRA:  mm.hexahedra.erase(mesh);  break;
        case WEDGES:     mm.wedges.erase(mesh);     break;
        case PYRAMIDS:   mm.pyramids.erase(mesh);   break;
        default: um_assert(!"Something is missing");
    }

    obj.broadcast(KILLED);
}

void XCF::kill_multimesh(const std::string & mm_name) {
    MultiMesh& mm = (*this)[mm_name];
    std::vector<std::string> to_kill;
    for (auto& [name, obj] : mm.polylines)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(ObjectId(POLYLINES, {mm_name, name}));

    for (auto& [name, obj] : mm.triangles)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(ObjectId(TRIANGLES, {mm_name, name}));


	std::map<std::string, MultiMesh>::erase(mm_name);
	ObjectId(MULTIMESH, mm_name).broadcast(KILLED);
    ObjectId(POINTSET, mm_name).broadcast(KILLED);
}
MultiMesh& XCF::add(std::string str) {
    ObjectId(MULTIMESH, str).broadcast(CREATED);
    return std::map<std::string, MultiMesh>::operator[](str);
}

MultiMesh::MeshAttr<Triangles, SurfaceAttributes>& XCF::add_triangles(std::string mm_name, std::string tri_name) {
    auto& collection = (*this)[mm_name].triangles;
    collection.try_emplace(tri_name);
    plop(collection.contains(tri_name));
    plop(tri_name);
    ObjectId obj(TRIANGLES, {mm_name, tri_name});
//    obj.show();
    obj.broadcast(CREATED);
    return collection[tri_name];
}

