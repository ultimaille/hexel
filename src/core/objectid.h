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

    struct BasicNode {
        std::vector<std::string> path = {};

        operator ObjectId() const {
            return ObjectId{ path };
        }
    };

    struct BasicCollection : BasicNode {
        ObjectId operator[](std::string s) const {
            return { append(path, s) };
        }
    };

    struct Path : BasicNode {
        BasicNode keys    = {{ "keys"    }};
        BasicNode mouse   = {{ "mouse"   }};
        BasicNode camera  = {{ "camera"  }};
        BasicNode context = {{ "context" }};

        BasicCollection modes   = {{{ "modes"   }}};
        BasicCollection layers  = {{{ "layers"  }}};
        BasicCollection panels  = {{{ "panels"  }}};
        BasicCollection shaders = {{{ "shaders" }}};

        struct XCF : BasicNode {
            struct MultiMesh : BasicNode {
                struct PointSet : BasicNode {
                    BasicCollection attr_points = { append(path, "attr_points") };
                } pointset = { append(path, "pointset") };

                struct PolyLines : BasicNode {
                    struct Mesh : BasicNode {
                        BasicCollection attr_points  = { append(path, "attr_points" ) };
                        BasicCollection attr_edges   = { append(path, "attr_edges" ) };
                    };
                    Mesh operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                struct Surfaces : BasicNode {
                    struct Mesh : BasicNode {
                        BasicCollection attr_points  = { append(path, "attr_points" ) };
                        BasicCollection attr_facets  = { append(path, "attr_facets" ) };
                        BasicCollection attr_corners = { append(path, "attr_corners") };
                    };
                    Mesh operator[](std::string s) const {
                        return { append(path, s) };
                    }
                };

                Surfaces triangles = { append(path, "triangles") };
                Surfaces quads     = { append(path, "quads"    ) };
                Surfaces polygons  = { append(path, "polygons" ) };

                struct Volumes : BasicNode {
                    struct Mesh : BasicNode {
                        BasicCollection attr_points  = { append(path, "attr_points" ) };
                        BasicCollection attr_cells   = { append(path, "attr_cells"  ) };
                        BasicCollection attr_facets  = { append(path, "attr_facets" ) };
                        BasicCollection attr_corners = { append(path, "attr_corners") };
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
        } xcf = {{{ "xcf"}}};
    };

    inline Path path() {
        return {};
    }

}

#endif //__OBJECTID_H__

