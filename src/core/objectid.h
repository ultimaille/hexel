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
    enum ObjectType { NA, KEYBOARD, MOUSE, CAMERA, PANEL, LAYER, MULTIMESH, 
        POINTSET, POLYLINES, TRIANGLES, QUADS, POLYGONS, TETRAHEDRA, HEXAHEDRA, WEDGES, PYRAMIDS,
        POINTSET_ATTR, POLYLINES_ATTR, TRIANGLES_ATTR, QUADS_ATTR, POLYGONS_ATTR, TETRAHEDRA_ATTR, HEXAHEDRA_ATTR, WEDGES_ATTR, PYRAMIDS_ATTR
    };
    enum EventType {
        CREATED, KILLED, UPDATED
    };
}

struct ObjectId {
   events::ObjectType path = events::NA;
    std::vector<std::string> names = {}; // multimesh name, mesh name, etc.

    ObjectId() = default;
    ObjectId(events::ObjectType path, std::vector<std::string> names={}) : path(path), names(names) {}
    ObjectId(events::ObjectType path, std::string name) : path(path), names(1, name) {}

//  ObjectId(std::vector<std::string> p) : path(p) {}
//  ObjectId(std::initializer_list<std::string> p) : path(p) {}

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

/*
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

*/
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
    return a.path == b.path && a.names == b.names;
}
















struct PathNode {
    const std::vector<std::string> path;

//  operator ObjectId() const {
//      return ObjectId{ path };
//  }

//  void broadcast(events::EventType e) {
//      ObjectId(*this).broadcast(e);
//  }
    [[deprecated("temporary compatibility conversion; migrate callers to Path")]]
        operator ObjectId() const;    

    protected:
    std::vector<std::string> concat(std::vector<std::string> path, std::string name) const {
        path.push_back(std::move(name));
        return path;
    }
};

struct PathCollection : PathNode {
    PathNode operator[](std::string s) const {
        return { concat(path, s) };
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

inline PathNode::operator ObjectId() const {
    const auto& p = path;

    if (p == std::vector<std::string>{"keys"})   return {events::KEYBOARD};
    if (p == std::vector<std::string>{"mouse"})  return {events::MOUSE};
    if (p == std::vector<std::string>{"camera"}) return {events::CAMERA};

    if (p.size() == 2 && p[0] == "layers")
        return {events::LAYER, p[1]};
    if (p.size() == 2 && p[0] == "panels")
        return {events::PANEL, p[1]};

    if (p.size() == 2 && p[0] == "xcf")
        return {events::MULTIMESH, p[1]};
    if (p.size() == 3 && p[0] == "xcf" && p[2] == "pointset")
        return {events::POINTSET, p[1]};

    if (p.size() == 4 && p[0] == "xcf") {
        events::ObjectType type = events::NA;
        if      (p[2] == "polylines")  type = events::POLYLINES;
        else if (p[2] == "triangles")  type = events::TRIANGLES;
        else if (p[2] == "quads")      type = events::QUADS;
        else if (p[2] == "polygons")   type = events::POLYGONS;
        else if (p[2] == "tetrahedra") type = events::TETRAHEDRA;
        else if (p[2] == "hexahedra")  type = events::HEXAHEDRA;
        else if (p[2] == "wedges")     type = events::WEDGES;
        else if (p[2] == "pyramids")   type = events::PYRAMIDS;

        um_assert(type != events::NA);
        return {type, {p[1], p[3]}};
    }

    // No legacy ObjectId equivalent, or an unsupported path shape.
    um_assert(!"Path cannot be converted to ObjectId");
    return {};
}

#endif //__OBJECTID_H__

