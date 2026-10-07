#pragma once

#include <core/basic.h>
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

        void load_from_path(std::string filename, bool connect = true);
        void load_geogram(std::string filename, bool connect = true);
        void save_to_path(std::string filename, bool geogram_compatible = false);
    };

    struct XCF: private std::map<std::string,MultiMesh> {

        using std::map<std::string, MultiMesh>::begin;
        using std::map<std::string, MultiMesh>::end;
        bool contains(const std::string& name);
        MultiMesh& operator[](std::string str);


        MultiMesh& add(std::string str);
        void add_mesh(ObjectId obj);

        void kill_mesh(ObjectId obj);
        void kill_multimesh(const std::string& name);

        std::string load_multimesh(std::string filename, bool connect = true);

        MultiMesh::MeshAttr<Triangles, SurfaceAttributes>& add_triangles(std::string mm_name, std::string tri_name);

    };
}

