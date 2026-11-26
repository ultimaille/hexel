#pragma once

#include <map>
#include <ultimaille/all.h>

struct MeshPath;

namespace UM {
    struct MultiMesh {
        template<class Mesh, class Attributes> struct MeshAttr {
            Mesh mesh;
            Attributes attributes;
        };

        PointSet pointset;
        PointSetAttributes pointset_attributes;

        std::map<std::string,MeshAttr<PolyLine,   PolyLineAttributes>> polylines;
        std::map<std::string,MeshAttr<Triangles,  SurfaceAttributes>>  triangles;
        std::map<std::string,MeshAttr<Quads,      SurfaceAttributes>>  quads;
        std::map<std::string,MeshAttr<Polygons,   SurfaceAttributes>>  polygons;
        std::map<std::string,MeshAttr<Tetrahedra, VolumeAttributes>>   tetrahedra;
        std::map<std::string,MeshAttr<Hexahedra,  VolumeAttributes>>   hexahedra;
        std::map<std::string,MeshAttr<Wedges,     VolumeAttributes>>   wedges;
        std::map<std::string,MeshAttr<Pyramids,   VolumeAttributes>>   pyramids;

        void load_from_path(std::string filename, bool connect = true);
        void load_geogram(std::string filename, bool connect = true);
        void save_to_path(std::string filename, bool geogram_compatible = false);

    };

    struct XCF: private std::map<std::string, MultiMesh> {

        using std::map<std::string, MultiMesh>::begin;
        using std::map<std::string, MultiMesh>::end;
        bool contains(const std::string& name);
        MultiMesh& operator[](std::string str);


        MultiMesh& add(std::string str);
        MeshPath add_polylines(const std::string& mm_name, const std::string& mesh_name);
        MeshPath add_triangles(const std::string& mm_name, const std::string& mesh_name);
        MeshPath add_quads(const std::string& mm_name, const std::string& mesh_name);
        MeshPath add_polygons(const std::string& mm_name, const std::string& mesh_name);
        MeshPath add_tetrahedra(const std::string& mm_name, const std::string& mesh_name);
        MeshPath add_hexahedra(const std::string& mm_name, const std::string& mesh_name);
        MeshPath add_wedges(const std::string& mm_name, const std::string& mesh_name);
        MeshPath add_pyramids(const std::string& mm_name, const std::string& mesh_name);

        void kill_mesh(MeshPath obj);
        void kill_multimesh(const std::string& name);

        std::string load_multimesh(std::string filename, bool connect = true);

        bool save(std::string filename, bool geogram_compatible);
        bool load(std::string filename);

    };
}

