#ifndef __OBJECTID_H__
#define __OBJECTID_H__

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <core/xcf.h>
#include <ultimaille/all.h>

namespace OID {

    struct ObjectId {
        using Path = std::vector<std::string>;
        Path path = {};

        ObjectId() = default;
        ObjectId(Path p) : path(p) {}
        ObjectId(std::initializer_list<std::string> p) : path(p) {}

        std::string debug_string() const {
            std::string out;
            for (size_t i = 0; i < path.size(); ++i) {
                if (i) out += ".";
                out += path[i];
            }
            return out;
        }
    };

    inline std::vector<std::string> append(std::vector<std::string> path, std::string name) {
        path.push_back(std::move(name));
        return path;
    }

    struct PathNode {
        std::vector<std::string> path = {};

        operator ObjectId() const {
            return ObjectId{ path };
        }
    };

    struct Path : PathNode {
        struct XCF : PathNode {
            struct MultiMesh : PathNode {
                struct Attributes : PathNode {
                    ObjectId operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                struct PointSet : PathNode {
                    Attributes PointAttribute  = { append(path, "PointAttribute") };
                } pointset = { append(path, "pointset") };

                struct PolyLines : PathNode {
                    struct Mesh : PathNode {
                        Attributes PointAttribute  = { append(path, "PointAttribute") };
                        Attributes EdgeAttribute   = { append(path, "EdgeAttribute" ) };
                    };
                    Mesh operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                struct Surfaces : PathNode {
                    struct Mesh : PathNode {
                        Attributes PointAttribute  = { append(path, "PointAttribute" ) };
                        Attributes FacetAttribute  = { append(path, "FacetAttribute" ) };
                        Attributes CornerAttribute = { append(path, "CornerAttribute") };
                    };
                    Mesh operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                Surfaces triangles = { append(path, "triangles") };
                Surfaces quads     = { append(path, "quads"    ) };
                Surfaces polygons  = { append(path, "polygons" ) };

                struct Volumes : PathNode {
                    struct Mesh : PathNode {
                        Attributes PointAttribute      = { append(path, "PointAttribute"     ) };
                        Attributes CellAttribute       = { append(path, "CellAttribute"      ) };
                        Attributes CellFacetAttribute  = { append(path, "CellFacetAttribute" ) };
                        Attributes CellCornerAttribute = { append(path, "CellCornerAttribute") };
                    };
                    Mesh operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                Volumes tetrahedra = { append(path, "tetrahedra") };
                Volumes hexahedra  = { append(path, "hexahedra" ) };
                Volumes wedges     = { append(path, "wedges "   ) };
                Volumes pyramids   = { append(path, "pyramids"  ) };
            };

            MultiMesh operator[](std::string s) const {
                return { append(path, s) };
            }
        } xcf = { append(path, "xcf") };
    };

    inline Path path() {
        return {};
    }

}

#endif //__OBJECTID_H__

