#include "path.h"

#include "events.h"
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

void Path::broadcast(EventTypes::EventType e) const {
    God::events.queue.emplace_back(Event(*this, e));
}

int Path::size() const {
    return static_cast<int>(path.size());
}

bool Path::has_prefix(const Path& other) const {
    return size() >= other.size() &&
        std::ranges::equal(
                other.path.begin(), other.path.end(),
                path.begin(), path.begin() + other.size()
                );
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
    const RootPath r = root; // TODO clean this

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
        return resolve_attribute(*this, (*slot)->attributes, mm_path.quads[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Polygons, SurfaceAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.polygons[mesh_name], path[5]).ptr;

    if (auto slot = std::get_if<MultiMesh::MeshAttr<Tetrahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.tetrahedra[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Hexahedra, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.hexahedra[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Wedges, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.wedges[mesh_name], path[5]).ptr;
    if (auto slot = std::get_if<MultiMesh::MeshAttr<Pyramids, VolumeAttributes>*>(&mesh_ptr))
        return resolve_attribute(*this, (*slot)->attributes, mm_path.pyramids[mesh_name], path[5]).ptr;

    return std::monostate{};
}

bool Path::valid() const {
    return std::visit(
        [](const auto val) -> bool {
            if constexpr (std::is_same_v<std::decay_t<decltype(val)>, std::monostate>)
                return false; // it is monostate
            else
                return val != nullptr; // check if the held pointer is non-null
            }, ptr()
            );
}

Path::operator PointSet*() const {
    auto* mm = as<MultiMesh>();
    return mm ? &mm->pointset : nullptr;
}

Path::operator PointSetAttributes*() const {
    auto* mm = as<MultiMesh>();
    return mm ? &mm->pointset_attributes : nullptr;
}

Path::operator PolyLineAttributes*() const {
    auto* attr = as<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>();
    return attr ? &attr->attributes : nullptr;
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

AttributeBase::TYPE AttributePath::kind() const {
    using TYPE = AttributeBase::TYPE;

    if (size() != 5 && size() != 6)
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

MultiMeshPath MeshPath::parent() const {
    um_assert((size() == 3 || size() == 4) && has_prefix(root.xcf));
    return MultiMeshPath{
        std::vector<std::string>(path.begin(), path.begin() + 2)
    };
}

MeshPath AttributePath::parent() const {
    um_assert((size() == 5 || size() == 6) && has_prefix(root.xcf));
    return MeshPath{
        std::vector<std::string>(path.begin(), path.end() - 2)
    };
}

