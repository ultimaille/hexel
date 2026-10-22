#ifndef __OBJECTID_H__
#define __OBJECTID_H__

#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <algorithm>
#include <ranges>

#include "xcf.h"
#include <ultimaille/all.h>

using namespace UM;

struct KeyboardState;
struct MouseState;
struct TrackBallCamera;
struct Panel;
struct Layer;

namespace events {
    enum EventType {
        CREATED, KILLED, UPDATED
    };
}

struct ObjectId {
    const std::vector<std::string> path = {};

    ObjectId() = default;
    ObjectId(std::vector<std::string> p) : path(p) {}
    ObjectId(std::initializer_list<std::string> p) : path(p) {}

    using ObjectRef = std::variant<
        std::reference_wrapper<KeyboardState>,
        std::reference_wrapper<MouseState>,
        std::reference_wrapper<TrackBallCamera>,
        std::reference_wrapper<Panel>,
        std::reference_wrapper<Layer>,
        std::reference_wrapper<MultiMesh>,
        std::reference_wrapper<MultiMesh::MeshAttr<PolyLine,   PolyLineAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Triangles,  SurfaceAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Quads,      SurfaceAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Polygons,   SurfaceAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Hexahedra,  VolumeAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Wedges,     VolumeAttributes>>,
        std::reference_wrapper<MultiMesh::MeshAttr<Pyramids,   VolumeAttributes>>
            >;

    std::optional<ObjectRef> ref();  // returns std::nullopt if path invalid

    operator KeyboardState&();
    operator MouseState&();
    operator TrackBallCamera&();
    operator Panel& ();
    operator Layer&();

    operator MultiMesh&();
    operator PointSet&();
    operator PointSetAttributes&();
    operator PolyLine&();
    operator PolyLineAttributes&();
    operator Triangles&();
    operator Quads&();
    operator Polygons&();
    operator SurfaceAttributes&();
    operator Tetrahedra&();
    operator Hexahedra&();
    operator Wedges&();
    operator Pyramids&();
    operator VolumeAttributes&();

    void broadcast(events::EventType e);

    inline bool is_prefix(const ObjectId& other) const {
        return path.size() <= other.path.size() && 
            std::ranges::equal(
                    path.begin(), path.end(),
                    other.path.begin(), other.path.begin() + path.size()
                    );
    }

    operator std::string() const { // TODO: remove this debug helper
        std::string out;
        for (size_t i = 0; i < path.size(); ++i) {
            out += "/" + path[i];
        }
        return out;
    }

    private:
    template<typename T> T& as() {
        std::optional<ObjectRef> v = ref();
        um_assert(v.has_value());
        auto* p = std::get_if<std::reference_wrapper<T>>(&*v);
        um_assert(p != nullptr);
        return p->get();
    }
};

inline bool operator==(const ObjectId& a, const ObjectId& b) {
    return a.path == b.path;
}

struct PathNode {
    const std::vector<std::string> path;

    operator ObjectId() const {
        return ObjectId{ path };
    }

    void broadcast(events::EventType e) {
        ObjectId(*this).broadcast(e);
    }

    protected:
    std::vector<std::string> concat(std::vector<std::string> path, std::string name) const {
        path.push_back(std::move(name));
        return path;
    }
};

struct PathCollection : PathNode {
    ObjectId operator[](std::string s) const {
        return { concat(path, s) };
    }
};

struct Path : PathNode {
    ObjectId keys    = {{ "keys"    }};
    ObjectId mouse   = {{ "mouse"   }};
    ObjectId camera  = {{ "camera"  }};
    ObjectId context = {{ "context" }};

    PathCollection modes   = {{{ "modes"   }}};
    PathCollection layers  = {{{ "layers"  }}};
    PathCollection panels  = {{{ "panels"  }}};
    PathCollection shaders = {{{ "shaders" }}};

    struct XCF : PathNode {
        struct MultiMesh : PathNode {
            struct PointSet : PathNode {
                PathCollection attr_points = { concat(path, "attr_points") };
            } pointset = { concat(path, "pointset") };

            struct PolyLines : PathNode {
                struct Mesh : PathNode {
                    PathCollection attr_points  = { concat(path, "attr_points" ) };
                    PathCollection attr_edges   = { concat(path, "attr_edges" ) };
                };
                Mesh operator[](std::string s) const {
                    return { concat(path, s) };
                }
            } polylines = { concat(path, "polylines") };

            struct Surfaces : PathNode {
                struct Mesh : PathNode {
                    PathCollection attr_points  = { concat(path, "attr_points" ) };
                    PathCollection attr_facets  = { concat(path, "attr_facets" ) };
                    PathCollection attr_corners = { concat(path, "attr_corners") };
                };
                Mesh operator[](std::string s) const {
                    return { concat(path, s) };
                }
            };

            Surfaces triangles = { concat(path, "triangles") };
            Surfaces quads     = { concat(path, "quads"    ) };
            Surfaces polygons  = { concat(path, "polygons" ) };

            struct Volumes : PathNode {
                struct Mesh : PathNode {
                    PathCollection attr_points  = { concat(path, "attr_points" ) };
                    PathCollection attr_cells   = { concat(path, "attr_cells"  ) };
                    PathCollection attr_facets  = { concat(path, "attr_facets" ) };
                    PathCollection attr_corners = { concat(path, "attr_corners") };
                };
                Mesh operator[](std::string s) const {
                    return { concat(path, s) };
                }
            };

            Volumes tetrahedra = { concat(path, "tetrahedra") };
            Volumes hexahedra  = { concat(path, "hexahedra" ) };
            Volumes wedges     = { concat(path, "wedges"    ) };
            Volumes pyramids   = { concat(path, "pyramids"  ) };
        };

        MultiMesh operator[](std::string s) const {
            return { concat(path, s) };
        }
    } xcf = {{{ "xcf" }}};
};

inline Path path() {
    return {};
}

#endif //__OBJECTID_H__

