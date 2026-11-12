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

    //  operator KeyboardState&();
    //  operator MouseState&();
    //  operator TrackBallCamera&();
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

        [[deprecated("temporary compatibility conversion; migrate callers to Path")]]
        operator Path() const;

        void broadcast(events::EventType e);

    /*
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















    struct Path {
        std::vector<std::string> path;

        void broadcast(events::EventType e) {
            ObjectId(*this).broadcast(e);
        }

        inline bool has_prefix(const Path& other) const {
            return size() >= other.size() &&
                std::ranges::equal(
                        other.path.begin(), other.path.end(),
                        path.begin(), path.begin() + other.size()
                        );
        }


        [[deprecated("temporary compatibility conversion; migrate callers to Path")]]
            operator ObjectId() const;

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

        ObjectPtr ptr() const;

        operator KeyboardState*() const;
        operator MouseState*() const;
        operator TrackBallCamera*() const;
        operator WindowContext*() const;
        operator Mode*() const;
        operator Panel*() const;
        operator Layer*() const;

        operator MultiMesh*() const;
        operator PointSet*() const;
        operator PointSetAttributes*() const;

        operator PolyLine*() const;
        operator PolyLineAttributes*() const;
        operator Triangles*() const;
        operator Quads*() const;
        operator Polygons*() const;
        operator SurfaceAttributes*() const;

        operator Tetrahedra*() const;
        operator Hexahedra*() const;
        operator Wedges*() const;
        operator Pyramids*() const;
        operator VolumeAttributes*() const;

        operator MultiMesh&() const;
        operator PointSet&() const;
        operator PointSetAttributes&() const;
        operator PolyLine&() const;
        operator PolyLineAttributes&() const;
        operator Triangles&() const;
        operator Quads&() const;
        operator Polygons&() const;
        operator SurfaceAttributes&() const;
        operator Tetrahedra&() const;
        operator Hexahedra&() const;
        operator Wedges&() const;
        operator Pyramids&() const;
        operator VolumeAttributes&() const;

        int size() const { return static_cast<int>(path.size()); }

        protected:
        std::vector<std::string> concat(std::vector<std::string> path, std::string name) const {
            path.push_back(std::move(name));
            return path;
        }

        template<typename T> T* as() const {
            ObjectPtr p = ptr();
            auto* result = std::get_if<T*>(&p);
            return result ? *result : nullptr;
        }
    };

    struct MultiMeshPath;

    struct MeshPath : Path {
        MultiMeshPath parent() const;
    };

struct AttributePath : Path {
    MeshPath parent() const;
};

    struct MultiMeshPath : Path {
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
    // TODO shaders?

    struct XCF : Path {
        MultiMeshPath operator[](std::string s) const {
            return { concat(path, s) };
        }
    } xcf = {{{ "xcf" }}};
};

inline RootPath root() {
    return {};
}

inline Path::operator ObjectId() const {
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

inline MultiMeshPath MeshPath::parent() const {
    um_assert(size() == 4 && has_prefix(root().xcf));
    return MultiMeshPath{
        std::vector<std::string>(path.begin(), path.end() - 2)
    };
}

inline MeshPath AttributePath::parent() const {
    um_assert(size() == 6 && has_prefix(root().xcf));
    return MeshPath{
        std::vector<std::string>(path.begin(), path.end() - 2)
    };
}

inline bool operator==(const Path& a, const Path& b) {
    return a.path == b.path;
}



inline ObjectId::operator Path() const {
    using events::ObjectType;

    auto make_path = [](std::initializer_list<std::string> parts) {
        return Path{std::vector<std::string>(parts)};
    };

    auto require_names = [this](std::size_t count) {
        um_assert(names.size() == count);
    };

    auto collection_for = [](ObjectType type) -> const char* {
        switch (type) {
        case events::POLYLINES:
        case events::POLYLINES_ATTR:  return "polylines";
        case events::TRIANGLES:
        case events::TRIANGLES_ATTR:  return "triangles";
        case events::QUADS:
        case events::QUADS_ATTR:      return "quads";
        case events::POLYGONS:
        case events::POLYGONS_ATTR:   return "polygons";
        case events::TETRAHEDRA:
        case events::TETRAHEDRA_ATTR: return "tetrahedra";
        case events::HEXAHEDRA:
        case events::HEXAHEDRA_ATTR:  return "hexahedra";
        case events::WEDGES:
        case events::WEDGES_ATTR:     return "wedges";
        case events::PYRAMIDS:
        case events::PYRAMIDS_ATTR:   return "pyramids";
        default:                      return nullptr;
        }
    };

    auto attribute_domain = [](const std::string& domain) -> const char* {
        if (domain == "points")  return "attr_points";
        if (domain == "edges")   return "attr_edges";
        if (domain == "facets")  return "attr_facets";
        if (domain == "corners") return "attr_corners";
        if (domain == "cells")   return "attr_cells";
        return nullptr;
    };

    switch (path) {
    case events::KEYBOARD:
        require_names(0);
        return make_path({"keys"});
    case events::MOUSE:
        require_names(0);
        return make_path({"mouse"});
    case events::CAMERA:
        require_names(0);
        return make_path({"camera"});

    case events::PANEL:
        require_names(1);
        return make_path({"panels", names[0]});
    case events::LAYER:
        require_names(1);
        return make_path({"layers", names[0]});
    case events::MULTIMESH:
        require_names(1);
        return make_path({"xcf", names[0]});
    case events::POINTSET:
        require_names(1);
        return make_path({"xcf", names[0], "pointset"});

    case events::POLYLINES:
    case events::TRIANGLES:
    case events::QUADS:
    case events::POLYGONS:
    case events::TETRAHEDRA:
    case events::HEXAHEDRA:
    case events::WEDGES:
    case events::PYRAMIDS: {
        require_names(2);
        const char* collection = collection_for(path);
        um_assert(collection != nullptr);
        return make_path({"xcf", names[0], collection, names[1]});
    }

    case events::POINTSET_ATTR: {
        // Legacy layout: {multimesh, attribute_type, domain, attribute_name}
        require_names(4);
        const char* domain = attribute_domain(names[2]);
        um_assert(domain == std::string("attr_points"));
        return make_path({"xcf", names[0], "pointset", domain, names[3]});
    }

    case events::POLYLINES_ATTR:
    case events::TRIANGLES_ATTR:
    case events::QUADS_ATTR:
    case events::POLYGONS_ATTR:
    case events::TETRAHEDRA_ATTR:
    case events::HEXAHEDRA_ATTR:
    case events::WEDGES_ATTR:
    case events::PYRAMIDS_ATTR: {
        // Legacy layout: {multimesh, mesh, attribute_type, domain, attribute_name}
        require_names(5);
        const char* collection = collection_for(path);
        const char* domain = attribute_domain(names[3]);
        um_assert(collection != nullptr);
        um_assert(domain != nullptr);
        return make_path(
            {"xcf", names[0], collection, names[1], domain, names[4]}
        );
    }

    default:
        um_assert(!"ObjectId has no corresponding Path");
        return {};
    }
}


#endif //__OBJECTID_H__

