#include "objectid.h"

#include "event.h"
#include "core.h"
#include "camera.h"
#include "picker.h"
#include "xcf.h"
#include "render_target.h"
#include "window.h"
#include "panels.h"
#include "layers.h"
#include "shaders.h"
#include "mode.h"

using namespace events;

void ObjectId::broadcast(EventType e) {
    God::events.queue.emplace_back(Event(*this, e));
}

std::optional<ObjectId::ObjectRef> ObjectId::ref() {
    switch (path) {
        case KEYBOARD:
            return std::ref(God::keys);

        case MOUSE:
            return std::ref(God::mouse);

        case CAMERA:
            return std::ref(God::camera);

        case PANEL:
            if (names.size() != 1) return std::nullopt;
            if (!God::panels.contains(names[0])) return std::nullopt;
            return std::ref(God::panels[names[0]]);

        case LAYER:
            if (names.size() != 1) return std::nullopt;
            if (!God::layers.contains(names[0])) return std::nullopt;
            return std::ref(God::layers[names[0]]);

        case MULTIMESH:
        case POINTSET: {
            if (names.size() != 1) return std::nullopt;
            std::string mm = names[0];
            if (!God::xcf.contains(mm)) return std::nullopt;
            return std::ref(God::xcf[mm]);
        }

        case POLYLINES:
        case TRIANGLES:
        case QUADS:
        case POLYGONS:
        case TETRAHEDRA:
        case HEXAHEDRA:
        case WEDGES:
        case PYRAMIDS: {
            if (names.size() != 2) return std::nullopt;
            std::string mm = names[0];
            std::string m  = names[1];
            if (!God::xcf.contains(mm)) return std::nullopt;

            auto& multimesh = God::xcf[mm];

            switch (path) {
                case POLYLINES:
                    if (!multimesh.polylines.contains(m)) return std::nullopt;
                    return std::ref(multimesh.polylines[m]);
                case TRIANGLES:
                    if (!multimesh.triangles.contains(m)) return std::nullopt;
                    return std::ref(multimesh.triangles[m]);
                case QUADS:
                    if (!multimesh.quads.contains(m)) return std::nullopt;
                    return std::ref(multimesh.quads[m]);
                case POLYGONS:
                    if (!multimesh.polygons.contains(m)) return std::nullopt;
                    return std::ref(multimesh.polygons[m]);
                case TETRAHEDRA:
                    if (!multimesh.tetrahedra.contains(m)) return std::nullopt;
                    return std::ref(multimesh.tetrahedra[m]);
                case HEXAHEDRA:
                    if (!multimesh.hexahedra.contains(m)) return std::nullopt;
                    return std::ref(multimesh.hexahedra[m]);
                case WEDGES:
                    if (!multimesh.wedges.contains(m)) return std::nullopt;
                    return std::ref(multimesh.wedges[m]);
                case PYRAMIDS:
                    if (!multimesh.pyramids.contains(m)) return std::nullopt;
                    return std::ref(multimesh.pyramids[m]);
                default:
                    um_assert(!"Something is missing here");
            }
        }

        default:
            return std::nullopt;
    }
    return std::nullopt;
}


/*
ObjectId::operator KeyboardState& () {
    return as<KeyboardState>();
}

ObjectId::operator MouseState& () {
    return as<MouseState>();
}

ObjectId::operator TrackBallCamera& () {
    return as<TrackBallCamera>();
}
*/

ObjectId::operator Panel& () {
    return as<Panel>();
}

ObjectId::operator Layer& () {
    return as<Layer>();
}

ObjectId::operator MultiMesh&() {
    return as<MultiMesh>();
}

ObjectId::operator PointSet&() {
    return as<MultiMesh>().pointset;
}

ObjectId::operator PointSetAttributes&() {
    return as<MultiMesh>().pointset_attributes;
}

ObjectId::operator PolyLine&() {
    auto &ma = as<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>();
    return ma.mesh;
}

ObjectId::operator PolyLineAttributes&() {
    auto &ma = as<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>();
    return ma.attributes;
}

ObjectId::operator Triangles&() {
    auto &ma = as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>();
    return ma.mesh;
}

ObjectId::operator Quads&() {
    auto &ma = as<MultiMesh::MeshAttr<Quads, SurfaceAttributes>>();
    return ma.mesh;
}

ObjectId::operator Polygons&() {
    auto &ma = as<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>>();
    return ma.mesh;
}

ObjectId::operator SurfaceAttributes&() {
    switch (path) {
        case TRIANGLES: return as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>().attributes;
        case QUADS:     return as<MultiMesh::MeshAttr<Quads,     SurfaceAttributes>>().attributes;
        case POLYGONS:  return as<MultiMesh::MeshAttr<Polygons,  SurfaceAttributes>>().attributes;
        default: um_assert(!"invalid cast");
    }
    static SurfaceAttributes dummy = {};
    return dummy;
}

ObjectId::operator Tetrahedra&() {
    auto &ma = as<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>>();
    return ma.mesh;
}

ObjectId::operator Hexahedra&() {
    auto &ma = as<MultiMesh::MeshAttr<Hexahedra, VolumeAttributes>>();
    return ma.mesh;
}

ObjectId::operator Wedges&() {
    auto &ma = as<MultiMesh::MeshAttr<Wedges, VolumeAttributes>>();
    return ma.mesh;
}

ObjectId::operator Pyramids&() {
    auto &ma = as<MultiMesh::MeshAttr<Pyramids, VolumeAttributes>>();
    return ma.mesh;
}

ObjectId::operator VolumeAttributes&() {
    switch (path) {
        case TETRAHEDRA: return as<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>>().attributes;
        case HEXAHEDRA:  return as<MultiMesh::MeshAttr<Hexahedra,  VolumeAttributes>>().attributes;
        case WEDGES:     return as<MultiMesh::MeshAttr<Wedges,     VolumeAttributes>>().attributes;
        case PYRAMIDS:   return as<MultiMesh::MeshAttr<Pyramids,   VolumeAttributes>>().attributes;
        default: um_assert(!"invalid cast");
    }
    static VolumeAttributes dummy = {};
    return dummy;
}




















template<class Collection> Path::ObjectPtr find_attribute_ptr(Collection& attributes, const std::string& name) {
    for (auto& attribute : attributes)
        if (attribute.name == name && attribute.ptr)
            return static_cast<ContainerBase*>(&*attribute.ptr);
    return std::monostate{};
}


struct AttributeResolution {
    AttributeBase::TYPE kind = AttributeBase::GENERIC;
    Path::ObjectPtr ptr = std::monostate{};
};

template<class Attributes, class MeshAttrPath> AttributeResolution resolve_attribute(const Path& attribute_path, Attributes& attributes, const MeshAttrPath& mesh_path, const std::string& name) {
    using TYPE = AttributeBase::TYPE;

    if (attribute_path.has_prefix(mesh_path.attr_points))
        return {TYPE::POINTS, find_attribute_ptr(attributes.points, name)};

    if constexpr (requires { mesh_path.attr_edges; attributes.edges; }) {
        if (attribute_path.has_prefix(mesh_path.attr_edges))
            return {TYPE::EDGES, find_attribute_ptr(attributes.edges, name)};
    }

    if constexpr (requires { mesh_path.attr_cells; attributes.cells; }) {
        if (attribute_path.has_prefix(mesh_path.attr_cells))
            return {TYPE::CELLS, find_attribute_ptr(attributes.cells, name)};
    }

    if constexpr (requires { mesh_path.attr_facets; }) {
        if (attribute_path.has_prefix(mesh_path.attr_facets)) {
            if constexpr (requires { attributes.cell_facets; })
                return {TYPE::CELLFACETS,
                    find_attribute_ptr(attributes.cell_facets, name)};
            else if constexpr (requires { attributes.facets; })
                return {TYPE::FACETS,
                    find_attribute_ptr(attributes.facets, name)};
        }
    }

    if constexpr (requires { mesh_path.attr_corners; }) {
        if (attribute_path.has_prefix(mesh_path.attr_corners)) {
            if constexpr (requires { attributes.cell_corners; })
                return {TYPE::CELLCORNERS,
                    find_attribute_ptr(attributes.cell_corners, name)};
            else if constexpr (requires { attributes.corners; })
                return {TYPE::CORNERS,
                    find_attribute_ptr(attributes.corners, name)};
        }
    }

    return {};
}


Path::ObjectPtr Path::ptr() const {
    const RootPath r = root();

    // exact match for single-segment root paths
    if (*this == r.keys)    return &God::keys;
    if (*this == r.mouse)   return &God::mouse;
    if (*this == r.camera)  return &God::camera;
    if (*this == r.context) return &God::context;

    // named root collections: layers/<name>, panels/<name>, modes/<name>
    if (size() == 2) {
        const std::string& name = path[1];
        if (has_prefix(r.layers) && God::layers.contains(name))
            return &God::layers[name];
        if (has_prefix(r.panels) && God::panels.contains(name))
            return &God::panels[name];
        if (has_prefix(r.modes) && God::modes.contains(name))
            return &God::modes[name];
        return std::monostate{};
    }

    // xcf paths all begin with xcf/<multimesh>.
    if (!has_prefix(r.xcf) || size() < 2)
        return std::monostate{};

    const auto& mm_name = path[1];
    if (!God::xcf.contains(mm_name))
        return std::monostate{};

    auto& mm = God::xcf[mm_name];
    const auto mm_path = r.xcf[mm_name];

    // exact multimesh path
    if (*this == mm_path)
        return &mm;

    // xcf/<mm>/pointset
    if (size() == 3 && *this == mm_path.pointset)
        return &mm; // TODO: no meshattr for pointsets, so I am forced to return multimesh instead

    // mesh paths:
    // xcf/<mm>/<mesh collection>/<mesh name>
    if (size() == 4) {
        const auto& mesh_name = path[3];
        if (has_prefix(mm_path.polylines) && mm.polylines.contains(mesh_name))
            return &mm.polylines[mesh_name];
        if (has_prefix(mm_path.triangles) && mm.triangles.contains(mesh_name))
            return &mm.triangles[mesh_name];                         
        if (has_prefix(mm_path.quads) && mm.quads.contains(mesh_name))
            return &mm.quads[mesh_name];                                                                      
        if (has_prefix(mm_path.polygons) && mm.polygons.contains(mesh_name))
            return &mm.polygons[mesh_name];
        if (has_prefix(mm_path.tetrahedra) && mm.tetrahedra.contains(mesh_name))                                   
            return &mm.tetrahedra[mesh_name];              
        if (has_prefix(mm_path.hexahedra) && mm.hexahedra.contains(mesh_name))
            return &mm.hexahedra[mesh_name];
        if (has_prefix(mm_path.wedges) && mm.wedges.contains(mesh_name))
            return &mm.wedges[mesh_name];
        if (has_prefix(mm_path.pyramids) && mm.pyramids.contains(mesh_name))                                                                                                                                                                                                     
            return &mm.pyramids[mesh_name];
        return std::monostate{};
    }

    // xcf/<mm>/pointset/attr_points/<attribute>
    if (size() == 5)
        return resolve_attribute(*this, mm.pointset_attributes, mm_path.pointset, path[4]).ptr;

    // mesh attribute paths:
    // xcf/<mm>/<collection>/<mesh>/<attribute collection>/<attribute name>
    if (size() != 6)
        return std::monostate{};

    // reuse mesh-path resolution for the first four segments
    MeshPath mesh_path{
        std::vector<std::string>(path.begin(), path.begin() + 4)
    };
    auto mesh_ptr = mesh_path.ptr();
    const auto& mesh_name = mesh_path.name();

    if (auto slot = std::get_if<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.polylines[mesh_name], path[5]).ptr;

    if (auto slot = std::get_if<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.triangles[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Quads, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.triangles[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.triangles[mesh_name], path[5]).ptr;

    if (auto slot = std::get_if<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.triangles[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Hexahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.triangles[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Wedges, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.triangles[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Pyramids, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.triangles[mesh_name], path[5]).ptr;

    return std::monostate{};
}


Path::operator KeyboardState*() const {
    return as<KeyboardState>();
}


Path::operator MouseState*() const {
    return as<MouseState>();
}

Path::operator TrackBallCamera*() const {
    return as<TrackBallCamera>();
}

Path::operator Panel*() const {
    return as<Panel>();
}

Path::operator Layer*() const {
    return as<Layer>();
}

Path::operator MultiMesh*() const {
    return as<MultiMesh>();
}

Path::operator PointSet*() const {
    auto* mm = as<MultiMesh>();
    return mm ? &mm->pointset : nullptr;
}

Path::operator PointSetAttributes*() const {
    auto* mm = as<MultiMesh>();
    return mm ? &mm->pointset_attributes : nullptr;
}

Path::operator PolyLine*() const {
    auto* attr = as<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator PolyLineAttributes*() const {
    auto* attr = as<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>();
    return attr ? &attr->attributes : nullptr;
}

Path::operator Triangles*() const {
    auto* attr = as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator Quads*() const {
    auto* attr = as<MultiMesh::MeshAttr<Quads, SurfaceAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator Polygons*() const {
    auto* attr = as<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator SurfaceAttributes*() const {
    if (path.size() < 3) return nullptr;

    if (path[2] == "triangles") {
        auto* attr = as<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>>();
        return attr ? &attr->attributes : nullptr;
    }
    if (path[2] == "quads") {
        auto* attr = as<MultiMesh::MeshAttr<Quads, SurfaceAttributes>>();
        return attr ? &attr->attributes : nullptr;
    }
    if (path[2] == "polygons") {
        auto* attr = as<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>>();
        return attr ? &attr->attributes : nullptr;
    }

    return nullptr;
}

Path::operator Tetrahedra*() const {
    auto* attr = as<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator Hexahedra*() const {
    auto* attr = as<MultiMesh::MeshAttr<Hexahedra, VolumeAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator Wedges*() const {
    auto* attr = as<MultiMesh::MeshAttr<Wedges, VolumeAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator Pyramids*() const {
    auto* attr = as<MultiMesh::MeshAttr<Pyramids, VolumeAttributes>>();
    return attr ? &attr->mesh : nullptr;
}

Path::operator VolumeAttributes*() const {
    if (path.size() < 3) return nullptr;

    if (path[2] == "tetrahedra") {
        auto* attr = as<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>>();
        return attr ? &attr->attributes : nullptr;
    }
    if (path[2] == "hexahedra") {
        auto* attr = as<MultiMesh::MeshAttr<Hexahedra, VolumeAttributes>>();
        return attr ? &attr->attributes : nullptr;
    }
    if (path[2] == "wedges") {
        auto* attr = as<MultiMesh::MeshAttr<Wedges, VolumeAttributes>>();
        return attr ? &attr->attributes : nullptr;
    }
    if (path[2] == "pyramids") {
        auto* attr = as<MultiMesh::MeshAttr<Pyramids, VolumeAttributes>>();
        return attr ? &attr->attributes : nullptr;
    }

    return nullptr;
}

Path::operator ContainerBase*() const {
    return as<ContainerBase>();
}

Path::operator PointSet&() const {
    PointSet *ptr = *this;
    um_assert(ptr != nullptr);
    return *ptr;
}


Path::operator PolyLine&() const {
    PolyLine *ptr = *this;
    um_assert(ptr != nullptr);
    return *ptr;
}

Path::operator Triangles&() const {
    Triangles *ptr = *this;
    um_assert(ptr != nullptr);
    return *ptr;
}

Path::operator PointSetAttributes&() const {
    PointSetAttributes *attr = *this;
    um_assert(attr != nullptr);
    return *attr;
}


Path::operator PolyLineAttributes&() const {
    PolyLineAttributes *attr = *this;
    um_assert(attr != nullptr);
    return *attr;
}

Path::operator SurfaceAttributes&() const {
    SurfaceAttributes *attr = *this;
    um_assert(attr != nullptr);
    return *attr;
}

Path::operator VolumeAttributes&() const {
    VolumeAttributes *attr = *this;
    um_assert(attr != nullptr);
    return *attr;
}

Path::operator Hexahedra&() const {
    Hexahedra *ptr = *this;
    um_assert(ptr != nullptr);
    return *ptr;
}


AttributeBase::TYPE AttributePath::kind() const {
    using TYPE = AttributeBase::TYPE;

    if (size() != 5 || size() != 6)
        return TYPE::GENERIC;

    const MeshPath mesh = parent();
    auto mesh_ptr = mesh.ptr();
    const MultiMeshPath multimesh = mesh.parent();

    if (size() == 5)
        return resolve_attribute(*this, God::xcf[multimesh.name()].pointset_attributes, multimesh.pointset, name()).kind;

    if (auto slot = std::get_if<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.polylines[mesh.name()], name()).kind;

    if (auto slot = std::get_if<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.triangles[mesh.name()], name()).kind;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Quads, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.quads[mesh.name()], name()).kind;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.polygons[mesh.name()], name()).kind;

    if (auto slot = std::get_if<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.tetrahedra[mesh.name()], name()).kind;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Hexahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.hexahedra[mesh.name()], name()).kind;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Wedges, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.wedges[mesh.name()], name()).kind;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Pyramids, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, multimesh.pyramids[mesh.name()], name()).kind;

    return TYPE::GENERIC;
}

