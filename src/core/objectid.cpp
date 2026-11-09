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

    // Pointset path and pointset attribute path:
    // xcf/<mm>/pointset
    // xcf/<mm>/pointset/attr_points/<attribute>
    if (size() == 3 && *this == mm_path.pointset)
        return &mm; // TODO: no meshattr for pointsets, so I am forced to return multimesh instead

    if (size() == 5 && has_prefix(mm_path.pointset.attr_points))
        return find_attribute_ptr(mm.pointset_attributes.points, path[4]);

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

    // mesh attribute paths:
    // xcf/<mm>/<collection>/<mesh>/<attribute collection>/<attribute name>
    if (size() != 6)
        return std::monostate{};

    // reuse mesh-path resolution for the first four segments
    MeshPath mesh_path{
        std::vector<std::string>(path.begin(), path.begin() + 4)
    };
    auto mesh_ptr = mesh_path.ptr();
    const auto& mesh_name = path[3];

    if (auto slot = std::get_if<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>*>(&mesh_ptr)) {
        auto& attributes = (*slot)->attributes;
        const auto attr_path = mm_path.polylines[mesh_name];
        if (has_prefix(attr_path.attr_points))
            return find_attribute_ptr(attributes.points, path[5]);
        if (has_prefix(attr_path.attr_edges))
            return find_attribute_ptr(attributes.edges, path[5]);
        return std::monostate{};
    }

    const auto resolve_surface_attribute = [&](SurfaceAttributes& attributes, const MultiMeshPath::Surfaces::Surface& mesh_attr_path) -> ObjectPtr {
        if (has_prefix(mesh_attr_path.attr_points))
            return find_attribute_ptr(attributes.points, path[5]);
        if (has_prefix(mesh_attr_path.attr_facets))
            return find_attribute_ptr(attributes.facets, path[5]);
        if (has_prefix(mesh_attr_path.attr_corners))
            return find_attribute_ptr(attributes.corners, path[5]);
        return std::monostate{};
    };

    if (auto slot = std::get_if<MultiMesh::MeshAttr<Triangles, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_surface_attribute((*slot)->attributes, mm_path.triangles[mesh_name]);
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Quads, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_surface_attribute((*slot)->attributes, mm_path.quads[mesh_name]);
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_surface_attribute((*slot)->attributes, mm_path.polygons[mesh_name]);

    const auto resolve_volume_attribute = [&](VolumeAttributes& attributes, const MultiMeshPath::Volumes::Volume& mesh_attr_path) -> ObjectPtr {
        if (has_prefix(mesh_attr_path.attr_points))
            return find_attribute_ptr(attributes.points, path[5]);
        if (has_prefix(mesh_attr_path.attr_cells))
            return find_attribute_ptr(attributes.cells, path[5]);
        if (has_prefix(mesh_attr_path.attr_facets))
            return find_attribute_ptr(attributes.cell_facets, path[5]);
        if (has_prefix(mesh_attr_path.attr_corners))
            return find_attribute_ptr(attributes.cell_corners, path[5]);
        return std::monostate{};
    };

    if (auto slot = std::get_if<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_volume_attribute((*slot)->attributes, mm_path.tetrahedra[mesh_name]);
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Hexahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_volume_attribute((*slot)->attributes, mm_path.hexahedra[mesh_name]);
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Wedges, VolumeAttributes>*>(&mesh_ptr))
        return resolve_volume_attribute((*slot)->attributes, mm_path.wedges[mesh_name]);
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Pyramids, VolumeAttributes>*>(&mesh_ptr))
        return resolve_volume_attribute((*slot)->attributes, mm_path.pyramids[mesh_name]);

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

