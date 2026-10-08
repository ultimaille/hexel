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
    um_assert(obj.path == POLYLINES || obj.path == TRIANGLES || obj.path == QUADS || obj.path == POLYGONS || obj.path == TETRAHEDRA || obj.path == HEXAHEDRA || obj.path == WEDGES || obj.path == PYRAMIDS);
    um_assert(obj.names.size() == 2);
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

void XCF::kill_multimesh(const std::string& mm_name) {
    MultiMesh& mm = (*this)[mm_name];
    std::vector<std::string> to_kill;
    for (auto& [name, obj] : mm.polylines)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(ObjectId(POLYLINES, { mm_name, name }));

    for (auto& [name, obj] : mm.triangles)  to_kill.push_back(name);
    for (auto name : to_kill) kill_mesh(ObjectId(TRIANGLES, { mm_name, name }));


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
    ObjectId obj(TRIANGLES, { mm_name, tri_name });
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
void XCF::add_mesh(ObjectId obj){
    um_assert(obj.names.size()==2);
    um_assert(contains(obj.names[0]));
    MultiMesh& mm = (*this)[obj.names[0]];

    switch (obj.path){
    case ObjectType::POLYLINES:   
        if (!mm.polylines.contains(obj.names[1]))  mm.polylines.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;

    case  ObjectType::TRIANGLES:  
        if (!mm.triangles.contains(obj.names[1]))  mm.triangles.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;
    case  ObjectType::QUADS:      
        if (!mm.quads.contains(obj.names[1]))  mm.quads.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;
    case  ObjectType::POLYGONS:   
        if (!mm.polygons.contains(obj.names[1]))  mm.polygons.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;

    case  ObjectType::TETRAHEDRA: 
        if (!mm.tetrahedra.contains(obj.names[1]))  mm.tetrahedra.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;
    case  ObjectType::HEXAHEDRA:  
        if (!mm.hexahedra.contains(obj.names[1]))  mm.hexahedra.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;
    case  ObjectType::PYRAMIDS:   
        if (!mm.pyramids.contains(obj.names[1]))  mm.pyramids.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;
    case  ObjectType::WEDGES:     
        if (!mm.wedges.contains(obj.names[1]))  mm.wedges.try_emplace(obj.names[1]); 
        obj.broadcast(CREATED); 
        return;
    }
    Log::error("Attempt to create a mesh that already exists");
}



