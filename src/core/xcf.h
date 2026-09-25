#pragma once

#include <map>
#include <ultimaille/all.h>

namespace UM {
    struct MultiMesh{
        template<class Mesh,class Attributes>
        struct MeshAttr{
            Mesh mesh;
            Attributes attributes;
        };


        PointSet pointset;
        PointSetAttributes pointset_attributes;

        std::map<std::string,MeshAttr<PolyLine,PolyLineAttributes>> polylines;

        std::map<std::string,MeshAttr<Triangles,SurfaceAttributes>> triangles;
        std::map<std::string,MeshAttr<Quads,SurfaceAttributes>> quads;
        std::map<std::string,MeshAttr<Polygons,SurfaceAttributes>> polygons;

        std::map<std::string,MeshAttr<Tetrahedra,VolumeAttributes>> tetrahedra;
        std::map<std::string,MeshAttr<Hexahedra,VolumeAttributes>> hexahedra;
        std::map<std::string,MeshAttr<Wedges,VolumeAttributes>> wedges;
        std::map<std::string,MeshAttr<Pyramids,VolumeAttributes>> pyramids;



        template<class T>
        void load_mesh(std::string mesh_file,std::string mesh_name,T& collection,bool connect){
            collection.try_emplace(mesh_name);
            auto& [mesh,attributes] = collection[mesh_name];
            attributes = read_by_extension(mesh_file,mesh);
            mesh.points = pointset;
            if(connect) mesh.connect();
        }

        void load_from_path(std::string filename,bool connect = true){
            std::filesystem::path path(filename);
            if(!std::filesystem::exists(path / "pointset.geogram")) return;

            std::cerr<<"Load pointset\n";
            pointset_attributes = read_by_extension((path / "pointset.geogram").string(),pointset);

            std::string collection_names[8] ={
                "polylines",
                "triangles","quads","polygons",
                "tetrahedra","hexahedra","wedges","pyramids"
            };
            for(int c = 0; c<8; c++){
                std::cerr<<"Load "<< collection_names[c] <<"\n";
                std::filesystem::path mesh_path = path / collection_names[c];
                if(!std::filesystem::exists(mesh_path)) continue;
                for(const auto& entry : std::filesystem::directory_iterator(mesh_path)){
                    if(!entry.is_regular_file()) continue;
                    std::string file = entry.path().string();
                    std::string name = entry.path().stem().string();
                    switch(c){
                    case 0: load_mesh(file,name,polylines,connect); break;
                    case 1: load_mesh(file,name,triangles,connect); break;
                    case 2: load_mesh(file,name,quads,connect); break;
                    case 3: load_mesh(file,name,polygons,connect); break;
                    case 4: load_mesh(file,name,tetrahedra,connect); break;
                    case 5: load_mesh(file,name,hexahedra,connect); break;
                    case 6: load_mesh(file,name,wedges,connect); break;
                    case 7: load_mesh(file,name,pyramids,connect); break;
                    }
                }
            }
        }


        void load_geogram(std::string filename,bool connect = true){
            if(!std::filesystem::exists(filename)) return;
            pointset_attributes = read_by_extension(filename,pointset);
            std::string collection_names[8] ={
                "polylines",
                "triangles","quads","polygons",
                "tetrahedra","hexahedra","wedges","pyramids"
            };
            for(int c = 0; c<8; c++){
                std::cerr<<"Load "<< collection_names[c] <<"\n";
                switch(c){
                case 0: load_mesh(filename,"polylines",polylines,connect); break;
                case 1: load_mesh(filename,"triangles",triangles,connect); break;
                case 2: load_mesh(filename,"quads",quads,connect); break;
                case 3: load_mesh(filename,"polygons",polygons,connect); break;
                case 4: load_mesh(filename,"tetrahedra",tetrahedra,connect); break;
                case 5: load_mesh(filename,"hexahedra",hexahedra,connect); break;
                case 6: load_mesh(filename,"wedges",wedges,connect); break;
                case 7: load_mesh(filename,"pyramids",pyramids,connect); break;
                }
            }
        }



        template<class T>
        void save_meshes(std::filesystem::path mesh_path,T& collection){
            if(collection.empty()) return;
            std::filesystem::create_directory(mesh_path);
            for(auto& [name,obj]:collection)
                write_by_extension((mesh_path.string() + std::string("/")+ name+std::string(".geogram")),obj.mesh,obj.attributes);
        }
        void save_to_path(std::string filename){
            std::filesystem::path path(filename);
            if(std::filesystem::exists(path)){
                std::cerr<<"Erase directory\n";
                std::filesystem::remove_all(path);
            }
            std::filesystem::create_directory(path);
            write_by_extension((path / "pointset.geogram").string(),pointset,pointset_attributes);

            save_meshes(path /"polylines",polylines);

            save_meshes(path /"triangles",triangles);
            save_meshes(path /"quads",quads);
            save_meshes(path /"polygons",polygons);

            save_meshes(path /"tetrahedra",tetrahedra);
            save_meshes(path /"hexahedra",hexahedra);
            save_meshes(path /"wedges",wedges);
            save_meshes(path /"pyramids",pyramids);
        }
    };

    struct XCF: public std::map<std::string,MultiMesh> {
        void load_multimesh(std::string filename,bool connect = true){
            std::string triname = std::filesystem::path(filename).stem().string();
            if(contains(triname))
                um_assert(false && "duplicate multimesh name");
            auto &multimesh = (*this)[triname];
            multimesh.load_geogram(filename,connect);
        }
        void erase(const std::string& name);
    };
}

