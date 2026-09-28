#pragma once
#include <vector>
#include <string>
#include <queue>
                                                     #

enum ObjectIdChunk{
    xcf,
        pointset,
        polylines,
        triangles,quads,polygons,
        tetrahedra,hexahedra,wedges,pyramids,
    layer,
    camera,
    mouse,
    key,
};


struct ObjectId {
    ObjectId(std::vector<ObjectIdChunk> hardpath, std::vector<std::string> softpath) : hardpath(hardpath),softpath(softpath) {}
    std::vector<ObjectIdChunk> hardpath;
    std::vector<std::string> softpath;
    bool is_a(ObjectIdChunk e);
    bool is(void* object);
};

enum EventType {
    CREATED, KILLED, UPDATED
};
struct Event {
    Event(ObjectId p_who, EventType p_what_happened):who(p_who), what_happened(p_what_happened){}
    ObjectId who;
    EventType what_happened;
};


struct EventManager {
    void push_back(Event event) {
        events.emplace(event);
    }
    void dispatch();
    std::queue<Event> events;
};