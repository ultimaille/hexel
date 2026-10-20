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

        operator std::string() const {
            std::string out;
            for (size_t i = 0; i < path.size(); ++i) {
                out += "/" + path[i];
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

    struct PathCollection : PathNode {
        ObjectId operator[](std::string s) const {
            return { append(path, s) };
        }
    };

    struct Path : PathNode {
        PathNode keys    = {{ "keys"    }};
        PathNode mouse   = {{ "mouse"   }};
        PathNode camera  = {{ "camera"  }};
        PathNode context = {{ "context" }};

        PathCollection modes   = {{{ "modes"   }}};
        PathCollection layers  = {{{ "layers"  }}};
        PathCollection panels  = {{{ "panels"  }}};
        PathCollection shaders = {{{ "shaders" }}};

        struct XCF : PathNode {
            struct MultiMesh : PathNode {
                struct PointSet : PathNode {
                    PathCollection attr_points = { append(path, "attr_points") };
                } pointset = { append(path, "pointset") };

                struct PolyLines : PathNode {
                    struct Mesh : PathNode {
                        PathCollection attr_points  = { append(path, "attr_points" ) };
                        PathCollection attr_edges   = { append(path, "attr_edges" ) };
                    };
                    Mesh operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                struct Surfaces : PathNode {
                    struct Mesh : PathNode {
                        PathCollection attr_points  = { append(path, "attr_points" ) };
                        PathCollection attr_facets  = { append(path, "attr_facets" ) };
                        PathCollection attr_corners = { append(path, "attr_corners") };
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
                        PathCollection attr_points  = { append(path, "attr_points" ) };
                        PathCollection attr_cells   = { append(path, "attr_cells"  ) };
                        PathCollection attr_facets  = { append(path, "attr_facets" ) };
                        PathCollection attr_corners = { append(path, "attr_corners") };
                    };
                    Mesh operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                Volumes tetrahedra = { append(path, "tetrahedra") };
                Volumes hexahedra  = { append(path, "hexahedra" ) };
                Volumes wedges     = { append(path, "wedges"    ) };
                Volumes pyramids   = { append(path, "pyramids"  ) };
            };

            MultiMesh operator[](std::string s) const {
                return { append(path, s) };
            }
        } xcf = {{{ "xcf"}}};
    };

    inline Path path() {
        return {};
    }

}

#endif //__OBJECTID_H__

