#ifndef __OBJECTID_H__
#define __OBJECTID_H__

#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <algorithm>
#include <ranges>
#include <type_traits>

#include "xcf.h"
#include <ultimaille/all.h>

using namespace UM;

struct KeyboardState;
struct MouseState;
struct TrackBallCamera;
struct WindowContext;
struct Panel;
struct Layer;
struct Path;
struct Mode;

namespace events {
    enum EventType {
        CREATED, KILLED, UPDATED
    };
}

struct Path {
    std::vector<std::string> path;
    void broadcast(events::EventType e) const;

    inline bool has_prefix(const Path& other) const {
        return size() >= other.size() &&
            std::ranges::equal(
                    other.path.begin(), other.path.end(),
                    path.begin(), path.begin() + other.size()
                    );
    }

    using ObjectPtr = std::variant<
        std::monostate, // unresolved path
        KeyboardState*,
        MouseState*,
        TrackBallCamera*,
        WindowContext*,
        Mode*,
        Panel*,
        Layer*,
        MultiMesh*,
        MultiMesh::MeshAttr<PolyLine,   PolyLineAttributes>*,
        MultiMesh::MeshAttr<Triangles,  SurfaceAttributes>*,
        MultiMesh::MeshAttr<Quads,      SurfaceAttributes>*,
        MultiMesh::MeshAttr<Polygons,   SurfaceAttributes>*,
        MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>*,
        MultiMesh::MeshAttr<Hexahedra,  VolumeAttributes>*,
        MultiMesh::MeshAttr<Wedges,     VolumeAttributes>*,
        MultiMesh::MeshAttr<Pyramids,   VolumeAttributes>*,
        ContainerBase*
            >;

    operator PointSet*() const;
    operator PointSetAttributes*() const;

//  operator PolyLine*() const;
    operator PolyLineAttributes*() const;
//  operator Triangles*() const;
//  operator Quads*() const;
//  operator Polygons*() const;
    operator SurfaceAttributes*() const;

//  operator Tetrahedra*() const;
//  operator Hexahedra*() const;
//  operator Wedges*() const;
//  operator Pyramids*() const;
    operator VolumeAttributes*() const;

    template<typename T>
        requires (
                std::is_same_v<T, PolyLine> ||
                std::is_same_v<T, Triangles> ||
                std::is_same_v<T, Quads> ||
                std::is_same_v<T, Polygons> ||
                std::is_same_v<T, Tetrahedra> ||
                std::is_same_v<T, Hexahedra> ||
                std::is_same_v<T, Wedges> ||
                std::is_same_v<T, Pyramids>
                )
        operator T*() const {
            using Attributes = decltype(empty_attr(std::declval<const T&>()));
            auto* attr = as<MultiMesh::MeshAttr<T, Attributes>>();
            return attr ? &attr->mesh : nullptr;
        }

    template<typename T>
        requires (
                std::is_same_v<T, KeyboardState> ||
                std::is_same_v<T, MouseState> ||
                std::is_same_v<T, TrackBallCamera> ||
                std::is_same_v<T, WindowContext> ||
                std::is_same_v<T, Mode> ||
                std::is_same_v<T, Panel> ||
                std::is_same_v<T, Layer> ||
                std::is_same_v<T, MultiMesh> ||
                std::is_same_v<T, ContainerBase>
                )
        operator T*() const {
            return as<T>();
        }

    template<typename T>
        requires (
                std::is_same_v<T, WindowContext> ||
                std::is_same_v<T, Mode> ||
                std::is_same_v<T, Panel> ||
                std::is_same_v<T, Layer> ||
                std::is_same_v<T, MultiMesh> ||
                std::is_same_v<T, PointSet> ||
                std::is_same_v<T, PointSetAttributes> ||
                std::is_same_v<T, PolyLine> ||
                std::is_same_v<T, PolyLineAttributes> ||
                std::is_same_v<T, Triangles> ||
                std::is_same_v<T, Quads> ||
                std::is_same_v<T, Polygons> ||
                std::is_same_v<T, SurfaceAttributes> ||
                std::is_same_v<T, Tetrahedra> ||
                std::is_same_v<T, Hexahedra> ||
                std::is_same_v<T, Wedges> ||
                std::is_same_v<T, Pyramids> ||
                std::is_same_v<T, VolumeAttributes>
                )
        operator T&() const {
            T* object = *this;
            um_assert(object != nullptr);
            return *object;
        }

    int size() const { return static_cast<int>(path.size()); }

    ObjectPtr ptr() const; // TODO move to protected?
    bool valid() const;

    protected:

    template<typename T> T* as() const {
        ObjectPtr p = ptr();
        auto* result = std::get_if<T*>(&p);
        return result ? *result : nullptr;
    }

    std::vector<std::string> concat(std::vector<std::string> path, std::string name) const {
        path.push_back(std::move(name));
        return path;
    }
};

struct MultiMeshPath;

struct MeshPath : Path {
    MultiMeshPath parent() const;
    const std::string &name() const { return path.back(); }
};

struct AttributePath : Path {
    MeshPath parent() const;
    AttributeBase::TYPE kind() const;
    const std::string &name() const { return path.back(); }
    const std::string &collection() const { return path[size()-2]; }
};

struct MultiMeshPath : Path {
    const std::string &name() const { return path.back(); }

    struct AttributeCollection : Path {
        AttributePath operator[](std::string s) const {
            return { concat(path, s) };
        }
    };

    struct PointSet : MeshPath {
        AttributeCollection attr_points = { concat(path, "attr_points") };
    } pointset = { concat(path, "pointset") };

    struct PolyLines : Path {
        struct PolyLine : MeshPath {
            AttributeCollection attr_points  = { concat(path, "attr_points") };
            AttributeCollection attr_edges   = { concat(path, "attr_edges" ) };
        };
        PolyLine operator[](std::string s) const {
            return { concat(path, s) };
        }
    } polylines = { concat(path, "polylines") };

    struct Surfaces : Path {
        struct Surface : MeshPath {
            AttributeCollection attr_points  = { concat(path, "attr_points" ) };
            AttributeCollection attr_facets  = { concat(path, "attr_facets" ) };
            AttributeCollection attr_corners = { concat(path, "attr_corners") };
        };
        Surface operator[](std::string s) const {
            return { concat(path, s) };
        }
    };

    Surfaces triangles = { concat(path, "triangles") };
    Surfaces quads     = { concat(path, "quads"    ) };
    Surfaces polygons  = { concat(path, "polygons" ) };

    struct Volumes : Path {
        struct Volume : MeshPath {
            AttributeCollection attr_points  = { concat(path, "attr_points" ) };
            AttributeCollection attr_cells   = { concat(path, "attr_cells"  ) };
            AttributeCollection attr_facets  = { concat(path, "attr_facets" ) };
            AttributeCollection attr_corners = { concat(path, "attr_corners") };
        };
        Volume operator[](std::string s) const {
            return { concat(path, s) };
        }
    };

    Volumes tetrahedra = { concat(path, "tetrahedra") };
    Volumes hexahedra  = { concat(path, "hexahedra" ) };
    Volumes wedges     = { concat(path, "wedges"    ) };
    Volumes pyramids   = { concat(path, "pyramids"  ) };
};

using PointSetPath = MultiMeshPath::PointSet;

struct RootPath : Path {
    Path keys    = {{ "keys"    }};
    Path mouse   = {{ "mouse"   }};
    Path camera  = {{ "camera"  }};
    Path context = {{ "context" }};

    struct PathCollection : Path {
        Path operator[](std::string s) const {
            return { concat(path, s) };
        }
    };

    PathCollection modes   = {{{ "modes"   }}};
    PathCollection layers  = {{{ "layers"  }}};
    PathCollection panels  = {{{ "panels"  }}};

    struct XCF : Path {
        MultiMeshPath operator[](std::string s) const {
            return { concat(path, s) };
        }
    } xcf = {{{ "xcf" }}};
};

inline bool operator==(const Path& a, const Path& b) {
    return a.path == b.path;
}

#endif //__OBJECTID_H__

