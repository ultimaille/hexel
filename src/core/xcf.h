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



        PointSet points;
        std::map<std::string, MeshAttr<Triangles, SurfaceAttributes>> triangles;

        void load(std::string filename, bool connect = true){
            std::filesystem::path path(filename);
            std::string triname = path.stem().string();
            if (triangles.contains(triname))
                um_assert(false && "duplicate mesh name");
            triangles.try_emplace(triname);
            auto& [tri, attributes] = triangles[triname];
            attributes = read_by_extension(filename, tri);
            if(connect) tri.connect();
        }

    };

    struct XCF: public std::map<std::string, MultiMesh> {
        void load_multimesh(std::string filename,bool connect = true){
            std::string triname = std::filesystem::path(filename).stem().string();
            if (contains(triname))
                um_assert(false && "duplicate multimesh name");
            auto &multimesh = (*this)[triname];
            multimesh.load(filename, connect);
        }
    };
}

