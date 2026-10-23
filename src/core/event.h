#pragma once
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <core/xcf.h>
#include <ultimaille/all.h>

using namespace UM;

struct KeyboardState;
struct MouseState;
struct TrackBallCamera;
struct Panel;
struct Layer;

// /!\ Note: ObjectId est devenu un moyen d'outrepasser le système de type, impression de manipuler un void*
// En effet intéressant d'avoir quelque chose qui permet de faire par exemple, xcf.get_by_id<Type>(objectid) 
// pour retrouver (multimesh / mesh / attribute) avec Type = {Multimesh, Mesh, Attribute}...
// on sais qu'on veux soit un mesh / multimesh / attributs, pas besoin d'avoir objectid qui wrap tout et n'importe quoi !
// je pense qu'il vaudrait mieux abstraire les maillages sous un type Mesh (ou std::variant de MeshAttr plutôt), attributes sous Attribute... pour passer en paramètre des objets avec des caractéristiques qui se recoupent
// J'ai beaucoup vu dans le code des wraps / unwraps d'object id... car objectid en paramètre alors même que pas nécessaire car on connait le type attendu par la fonction
// Qu'on veuille obtenir un objet lambda dans l'application ok, mais passer des objects id en parametre pas nécessaire, inutile, casse le système de type...

// Serait suffisant comme interface sur le xcf: 
// - xcf.find<Layer>("layer_name" | layer_id) -> Layer
// - xcf.find<Panel>("panel_name") -> Panel
// - xcf.find<Mesh>("mm_name", "mesh_name") -> MeshAttr variant
// aucune raison de devoir trouver un attribut depuis le xcf plutôt à partir du mesh.

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
    ObjectId() = default;
    ObjectId(events::ObjectType path, std::vector<std::string> names={}) : path(path), names(names) {}
    ObjectId(events::ObjectType path, std::string name) : path(path), names(1, name) {}

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

    events::ObjectType path = events::NA;
    std::vector<std::string> names = {}; // multimesh name, mesh name, etc.

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

inline bool operator==(const ObjectId& o, const events::ObjectType& p) {
    return o.path == p && !o.names.size();
}

inline bool operator==(const events::ObjectType& p, const ObjectId& o) {
    return o.path == p && !o.names.size();
}

struct Event {
    ObjectId who;
    events::EventType what_happened;
};

struct EventManager {
    void dispatch();
    std::vector<Event> queue;
};                                                           

