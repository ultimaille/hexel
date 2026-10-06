#pragma once

#include "basic.h"
#include <map>
#include <ultimaille/all.h>
#include <fstream>
struct ObjectId;

namespace UM {
    struct MultiMesh {
        template<class Mesh,class Attributes>
        struct MeshAttr {
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

        static std::string collection_names[8] ;


        template<class T>
        void load_mesh(std::string mesh_file,std::string mesh_name,T& collection,bool connect){
            collection.try_emplace(mesh_name);
            auto& [mesh,attributes] = collection[mesh_name];
            attributes = read_by_extension(mesh_file,mesh);
            if (mesh.nverts()==0)
                attributes.points = {};
            mesh.points = pointset;
            if(connect) mesh.connect();
        }

        void load_from_path(std::string filename,bool connect = true){
            std::filesystem::path path(filename);
            if(!std::filesystem::exists(path / "pointset.geogram")) return;

            std::cerr<<"Load pointset\n";
            pointset_attributes = read_by_extension((path / "pointset.geogram").string(),pointset);


            for(int c = 0; c<8; c++){
                std::filesystem::path mesh_path = path / collection_names[c];
                if(!std::filesystem::exists(mesh_path)) continue;
                for(const auto& entry : std::filesystem::directory_iterator(mesh_path)){
                    if(!entry.is_regular_file()) continue;
                    std::string file = entry.path().string();
                    std::string name = entry.path().stem().string();
                    if (name == entry.path().filename().string()) name = "";
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

            for(int c = 0; c<8; c++){
                //std::cerr<<"Load "<< collection_names[c] <<"\n";
                switch(c){
                case 0: load_mesh(filename,"",polylines,connect); break;
                case 1: load_mesh(filename,"",triangles,connect); break;
                case 2: load_mesh(filename,"",quads,connect); break;
                case 3: load_mesh(filename,"",polygons,connect); break;
                case 4: load_mesh(filename,"",tetrahedra,connect); break;
                case 5: load_mesh(filename,"",hexahedra,connect); break;
                case 6: load_mesh(filename,"",wedges,connect); break;
                case 7: load_mesh(filename,"",pyramids,connect); break;
                }
            }
            std::string primitives_loaded= "";
            if(polylines[""].mesh.nedges()==0)                                     polylines.erase("");
            else primitives_loaded+="polylines ";

            if(triangles[""].mesh.nfacets()==0)                                    triangles.erase("");
            else primitives_loaded+="triangles ";
            if(quads[""].mesh.nfacets()==0)                                            quads.erase("");
            else primitives_loaded+="quads ";
            if(polygons[""].mesh.nfacets()==triangles[""].mesh.nfacets())  polygons.erase("");
            else 
            if(polygons[""].mesh.nfacets()==quads[""].mesh.nfacets())          polygons.erase("");
            else primitives_loaded+="Polygons ";

            if(tetrahedra[""].mesh.ncells()==0)                                   tetrahedra.erase("");
            else primitives_loaded+="tetrahedra ";
            if(hexahedra[""].mesh.ncells()==0)                                     hexahedra.erase("");
            else primitives_loaded+="hexahedra ";
            if (wedges[""].mesh.ncells() == 0)                                       wedges.erase("");
            else primitives_loaded += "wedges ";
            if (pyramids[""].mesh.ncells() == 0)                                       pyramids.erase("");
            else primitives_loaded += "pyramid ";
            Log::add("primitives loaded in .geogram: " +primitives_loaded);
        }



        template<class T>
        void save_meshes(std::filesystem::path mesh_path,T& collection,bool geogram_compatible=false){
            if(collection.empty()) return;
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
        void save_to_path(std::string filename, bool geogram_compatible = false){
            std::filesystem::path path(filename);
            if(std::filesystem::exists(path)){
                std::cerr<<"Erase directory\n";
                std::filesystem::remove_all(path);
            }
            std::ofstream file(path.string() + ".mm");
            std::filesystem::create_directory(path);
            write_by_extension((path / "pointset.geogram").string(),pointset,pointset_attributes);

            save_meshes(path /"polylines"   ,polylines  ,geogram_compatible);

            save_meshes(path /"triangles"   ,triangles  ,geogram_compatible);
            save_meshes(path /"quads"       ,quads      ,geogram_compatible);
            save_meshes(path /"polygons"    ,polygons   ,geogram_compatible);

            save_meshes(path /"tetrahedra"  ,tetrahedra ,geogram_compatible);
            save_meshes(path /"hexahedra"   ,hexahedra  ,geogram_compatible);
            save_meshes(path /"wedges"      ,wedges     ,geogram_compatible);
            save_meshes(path /"pyramids"    ,pyramids   ,geogram_compatible);
        }
    };

    struct XCF: private std::map<std::string,MultiMesh> {

        using std::map<std::string, MultiMesh>::begin;
        using std::map<std::string, MultiMesh>::end;
        bool contains(const std::string& name){ return std::map<std::string, MultiMesh>::contains(name); }

        MultiMesh& operator[](std::string str) {
            if (!contains(str)) abort();
            return  std::map<std::string, MultiMesh>::operator[](str);
        }
        MultiMesh& add(std::string str);
        std::string load_multimesh(std::string filename,bool connect = true){
            std::string triname = std::filesystem::path(filename).stem().string();
            while (contains(triname)) triname += "_";
            MultiMesh& multimesh = add(triname);
            if (std::filesystem::is_directory(filename))
                multimesh.load_from_path(filename, connect);
            else
            multimesh.load_geogram(filename,connect);
            return triname;
        }
        void kill_multimesh(const std::string& name);
        void kill_mesh(ObjectId obj);

        MultiMesh::MeshAttr<Triangles, SurfaceAttributes>& add_triangles(std::string mm_name, std::string tri_name);

    };
}

