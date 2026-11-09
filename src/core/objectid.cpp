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































template<class Collection>
std::optional<std::reference_wrapper<ContainerBase>>
find_attribute(Collection& attributes, const std::string& name) {
    for (auto& attribute : attributes) {
        if (attribute.name == name && attribute.ptr) {
            return std::ref(static_cast<ContainerBase&>(*attribute.ptr));
        }
    }
    return std::nullopt;
}

std::optional<Path::ObjectRef> Path::ref() const {
    const RootPath r = root();

    if (*this == r.keys)    return std::ref(God::keys);
    if (*this == r.mouse)   return std::ref(God::mouse);
    if (*this == r.camera)  return std::ref(God::camera);
    if (*this == r.context) return std::ref(God::context);

    if (path.size() == 2) {
        const std::string& name = path.back();
        if (has_prefix(r.layers)) {
            if (God::layers.contains(name))
                return std::ref(God::layers[name]);
            return std::nullopt;
        }

        if (has_prefix(r.panels)) {
            if (God::panels.contains(name))
                return std::ref(God::panels[name]);
            return std::nullopt;
        }

        if (has_prefix(r.modes)) {
            if (God::modes.contains(name))
                return std::ref(God::modes[name]);
            return std::nullopt;
        }
    }

    if (!has_prefix(r.xcf) || path.size() < 2)
        return std::nullopt;

    const std::string& mm_name = path[1];
    if (!God::xcf.contains(mm_name))
        return std::nullopt;

    MultiMesh& mm = God::xcf[mm_name];
    const MultiMeshPath mm_path = r.xcf[mm_name];

    if (*this == mm_path)
        return std::ref(mm);

    if (has_prefix(mm_path.pointset)) {
        if (path.size()==2)
            return std::ref(mm.pointset);

        if (path.size()==4 && has_prefix(mm_path.pointset.attr_points))
            return find_attribute(mm.pointset_attributes.points, path.back());

        return std::nullopt;
    }

    // resolve a mesh path, such as xcf/<multimesh>/triangles/<mesh>.
    if (path.size() == 4) {
        const std::string& meshattr_name = path.back();
        if (has_prefix(mm_path.polylines) && mm.polylines.contains(meshattr_name))
                return mm.polylines[meshattr_name];

        if (has_prefix(mm_path.triangles) && mm.triangles.contains(meshattr_name))
                return mm.triangles[meshattr_name];

        if (has_prefix(mm_path.quads) && mm.quads.contains(meshattr_name))
                return mm.quads[meshattr_name];

        if (has_prefix(mm_path.polygons) && mm.polygons.contains(meshattr_name))
                return mm.polygons[meshattr_name];

        if (has_prefix(mm_path.tetrahedra) && mm.tetrahedra.contains(meshattr_name))
                return mm.tetrahedra[meshattr_name];

        if (has_prefix(mm_path.hexahedra) && mm.hexahedra.contains(meshattr_name))
                return mm.hexahedra[meshattr_name];

        if (has_prefix(mm_path.wedges) && mm.wedges.contains(meshattr_name))
                return mm.wedges[meshattr_name];

        if (has_prefix(mm_path.pyramids) && mm.pyramids.contains(meshattr_name))
                return mm.pyramids[meshattr_name];

        return std::nullopt;
    }

    // attribute paths have one additional segment after their collection:
    // xcf/<multimesh>/<collection>/<mesh>/<attribute collection>/<attribute>
    if (path.size() != 6)
        return std::nullopt;

    MeshPath mesh_path{
        std::vector<std::string>(path.begin(), path.begin() + 4)
    };

    auto mesh_ref = mesh_path.ref();
    if (!mesh_ref)
        return std::nullopt;

    const std::string& meshattr_name = path[3];

    auto* polyline_ref = std::get_if<std::reference_wrapper<MultiMesh::MeshAttr<PolyLine, PolyLineAttributes>>>(&*mesh_ref);
    if (polyline_ref) {
        auto& attributes = polyline_ref->get().attributes;
        if (has_prefix(r.xcf[mm_name].polylines[meshattr_name].attr_points))
            return find_attribute(attributes.points, path[5]);
        if (has_prefix(r.xcf[mm_name].polylines[meshattr_name].attr_edges))
            return find_attribute(attributes.edges, path[5]);
        return std::nullopt;
    }



    return std::nullopt;
}



