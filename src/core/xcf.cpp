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


const std::string collection_names[8] = {
"polylines",
"triangles","quads","polygons",
"tetrahedra","hexahedra","wedges","pyramids"
};






template<class T>
void load_mesh(PointSet& pointset, std::string mesh_file, std::string mesh_name, T& collection, bool connect){
    collection.try_emplace(mesh_name);
    auto& [mesh, attributes] = collection[mesh_name];
    attributes = read_by_extension(mesh_file, mesh);
    if (mesh.nverts() == 0)
        attributes.points = {};
    mesh.points = pointset;
    if (connect) mesh.connect();
}

void MultiMesh::load_from_path(std::string filename, bool connect){
    std::filesystem::path path(filename);
    if (!std::filesystem::exists(path / "pointset.geogram")) return;

    std::cerr << "Load pointset\n";
    pointset_attributes = read_by_extension((path / "pointset.geogram").string(), pointset);


    for (int c = 0; c < 8; c++){
        std::filesystem::path mesh_path = path / collection_names[c];
        if (!std::filesystem::exists(mesh_path)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(mesh_path)){
            if (!entry.is_regular_file()) continue;
            std::string file = entry.path().string();
            std::string name = entry.path().stem().string();
            if (name == entry.path().filename().string()) name = "";
            switch (c){
            case 0: load_mesh(pointset, file, name, polylines, connect); break;
            case 1: load_mesh(pointset, file, name, triangles, connect); break;
            case 2: load_mesh(pointset, file, name, quads, connect); break;
            case 3: load_mesh(pointset, file, name, polygons, connect); break;
            case 4: load_mesh(pointset, file, name, tetrahedra, connect); break;
            case 5: load_mesh(pointset, file, name, hexahedra, connect); break;
            case 6: load_mesh(pointset, file, name, wedges, connect); break;
            case 7: load_mesh(pointset, file, name, pyramids, connect); break;
            }
        }
    }
}


void MultiMesh::load_geogram(std::string filename, bool connect){
    if (!std::filesystem::exists(filename)) return;
    pointset_attributes = read_by_extension(filename, pointset);

    for (int c = 0; c < 8; c++){
        switch (c){
        case 0: load_mesh(pointset, filename, "", polylines, connect); break;
        case 1: load_mesh(pointset, filename, "", triangles, connect); break;
        case 2: load_mesh(pointset, filename, "", quads, connect); break;
        case 3: load_mesh(pointset, filename, "", polygons, connect); break;
        case 4: load_mesh(pointset, filename, "", tetrahedra, connect); break;
        case 5: load_mesh(pointset, filename, "", hexahedra, connect); break;
        case 6: load_mesh(pointset, filename, "", wedges, connect); break;
        case 7: load_mesh(pointset, filename, "", pyramids, connect); break;
        }
    }
    std::string primitives_loaded = "";
    if (polylines[""].mesh.nedges() == 0)                             polylines.erase("");
    else primitives_loaded += "polylines ";

    if (triangles[""].mesh.nfacets() == 0)                            triangles.erase("");
    else primitives_loaded += "triangles ";
    if (quads[""].mesh.nfacets() == 0)                                quads.erase("");
    else primitives_loaded += "quads ";
    if (!triangles.empty() && !polygons.empty() && polygons[""].mesh.nfacets() == triangles[""].mesh.nfacets())  polygons.erase("");
    else
        if (!quads.empty() && !polygons.empty() && polygons[""].mesh.nfacets() == quads[""].mesh.nfacets())  polygons.erase("");
        else primitives_loaded += "Polygons ";

    if (tetrahedra[""].mesh.ncells() == 0)                            tetrahedra.erase("");
    else primitives_loaded += "tetrahedra ";
    if (hexahedra[""].mesh.ncells() == 0)                             hexahedra.erase("");
    else primitives_loaded += "hexahedra ";
    if (wedges[""].mesh.ncells() == 0)                                wedges.erase("");
    else primitives_loaded += "wedges ";
    if (pyramids[""].mesh.ncells() == 0)                              pyramids.erase("");
    else primitives_loaded += "pyramid ";
    plop(triangles.size());

    Log::add("primitives loaded in .geogram: " + primitives_loaded);
}



template<class T>
void save_meshes(PointSet& pointset, std::filesystem::path mesh_path, T& collection, bool geogram_compatible = false){
    if (collection.empty()) return;
    std::filesystem::create_directory(mesh_path);
    for (auto& [name, obj] : collection) {
        PointSet empty;
        if (obj.attributes.points.empty() && !geogram_compatible)
            obj.mesh.points = empty;

        auto path = (mesh_path.string() + std::string("/") + name + std::string(".geogram"));
        write_by_extension(path, obj.mesh, obj.attributes);
        if (obj.attributes.points.empty() && !geogram_compatible)
            obj.mesh.points = pointset;
    }
}
void MultiMesh::save_to_path(std::string filename, bool geogram_compatible){
    std::filesystem::path path(filename);
    if (std::filesystem::exists(path)){
        std::cerr << "Erase directory\n";
        std::filesystem::remove_all(path);
    }
    std::ofstream file(path.string() + ".mm");
    std::filesystem::create_directory(path);
    write_by_extension((path / "pointset.geogram").string(), pointset, pointset_attributes);

    save_meshes(pointset, path / "polylines", polylines, geogram_compatible);

    save_meshes(pointset, path / "triangles", triangles, geogram_compatible);
    save_meshes(pointset, path / "quads", quads, geogram_compatible);
    save_meshes(pointset, path / "polygons", polygons, geogram_compatible);

    save_meshes(pointset, path / "tetrahedra", tetrahedra, geogram_compatible);
    save_meshes(pointset, path / "hexahedra", hexahedra, geogram_compatible);
    save_meshes(pointset, path / "wedges", wedges, geogram_compatible);
    save_meshes(pointset, path / "pyramids", pyramids, geogram_compatible);
}


using namespace events;



void XCF::kill_mesh(ObjectId obj) {
    um_assert(obj.path.size() == 4);
    auto ref = obj.ref();
    um_assert(ref.has_value());

    std::string mm_name = obj.path[1];
    std::string mesh = obj.path[3];
    std::string mesh_type = obj.path[2];

    MultiMesh& mm = (*this)[mm_name];

    if (ObjectId(path().xcf[mm_name].polylines).is_prefix(obj)) {
        mm.polylines.erase(mesh);
    }  else if (ObjectId(path().xcf[mm_name].triangles).is_prefix(obj)) {
        mm.triangles.erase(mesh);
    }  else if (ObjectId(path().xcf[mm_name].quads).is_prefix(obj)) {
        mm.quads.erase(mesh);
    }  else if (ObjectId(path().xcf[mm_name].polygons).is_prefix(obj)) {
        mm.polygons.erase(mesh);
    }  else if (ObjectId(path().xcf[mm_name].tetrahedra).is_prefix(obj)) {
        mm.tetrahedra.erase(mesh);
    }  else if (ObjectId(path().xcf[mm_name].hexahedra).is_prefix(obj)) {
        mm.hexahedra.erase(mesh);
    }  else if (ObjectId(path().xcf[mm_name].wedges).is_prefix(obj)) {
        mm.wedges.erase(mesh);
    }  else if (ObjectId(path().xcf[mm_name].pyramids).is_prefix(obj)) {
        mm.pyramids.erase(mesh);
    }  else {
        um_assert(!"Something is missing");
    }
    obj.broadcast(KILLED);
}

void XCF::kill_multimesh(const std::string& mm_name) {
    MultiMesh& mm = (*this)[mm_name];
    std::vector<std::string> to_kill;
    for (auto& [name, obj] : mm.polylines)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(path().xcf[mm_name].polylines[name]);

    for (auto& [name, obj] : mm.triangles)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(path().xcf[mm_name].triangles[name]);


    std::map<std::string, MultiMesh>::erase(mm_name);
    path().xcf[mm_name].broadcast(KILLED);
    path().xcf[mm_name].pointset.broadcast(KILLED);
}
MultiMesh& XCF::add(std::string str) {
    path().xcf[str].broadcast(CREATED);
    return std::map<std::string, MultiMesh>::operator[](str);
}

MultiMesh::MeshAttr<Triangles, SurfaceAttributes>& XCF::add_triangles(std::string mm_name, std::string tri_name) {
    auto& collection = (*this)[mm_name].triangles;
    collection.try_emplace(tri_name);
    plop(collection.contains(tri_name));
    plop(tri_name);
    ObjectId obj = path().xcf[mm_name].triangles[tri_name];
    //    obj.show();
    obj.broadcast(CREATED);
    return collection[tri_name];
}
std::string XCF::load_multimesh(std::string filename, bool connect ){
    std::string mm_name = std::filesystem::path(filename).stem().string();
    while (contains(mm_name)) mm_name += "_";
    MultiMesh& multimesh = add(mm_name);
    if (std::filesystem::is_directory(filename))
        multimesh.load_from_path(filename, connect);
    else
        multimesh.load_geogram(filename, connect);
    return mm_name;
}

bool XCF::contains(const std::string& name){ 
    return std::map<std::string, MultiMesh>::contains(name); 
}

MultiMesh& XCF::operator[](std::string str) {
    if (!contains(str)) abort();
    return  std::map<std::string, MultiMesh>::operator[](str);
}

void XCF::add_mesh(ObjectId obj) {
    um_assert(obj.path.size() == 4);

    std::string mm_name = obj.path[1];
    um_assert(contains(mm_name));
    std::string mesh = obj.path[3];
    std::string mesh_type = obj.path[2];
    MultiMesh& mm = (*this)[mm_name];

    if (ObjectId(path().xcf[mm_name].polylines).is_prefix(obj)) {
        if (!mm.polylines.contains(mesh))
            mm.polylines.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else if (ObjectId(path().xcf[mm_name].triangles).is_prefix(obj)) {
        if (!mm.triangles.contains(mesh))
            mm.triangles.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else if (ObjectId(path().xcf[mm_name].quads).is_prefix(obj)) {
        if (!mm.quads.contains(mesh))
            mm.quads.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else if (ObjectId(path().xcf[mm_name].polygons).is_prefix(obj)) {
        if (!mm.polygons.contains(mesh))
            mm.polygons.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else if (ObjectId(path().xcf[mm_name].tetrahedra).is_prefix(obj)) {
        if (!mm.tetrahedra.contains(mesh))
            mm.tetrahedra.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else if (ObjectId(path().xcf[mm_name].hexahedra).is_prefix(obj)) {
        if (!mm.hexahedra.contains(mesh))
            mm.hexahedra.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else if (ObjectId(path().xcf[mm_name].wedges).is_prefix(obj)) {
        if (!mm.wedges.contains(mesh))
            mm.wedges.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else if (ObjectId(path().xcf[mm_name].pyramids).is_prefix(obj)) {
        if (!mm.pyramids.contains(mesh))
            mm.pyramids.try_emplace(mesh); 
        obj.broadcast(CREATED); 
        return;
    }  else {
        um_assert(!"Something is missing");
    }
    Log::error("Attempt to create a mesh that already exists");
}



