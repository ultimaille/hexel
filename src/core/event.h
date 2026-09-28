#pragma once
#include <vector>
#include <string>
#include <queue>
                                                     #

enum EventIdChunk{
    xcf,
        pointset,
        triangles,quads,polygons,
        tetrahedra,hexahedra,wedges,pyramids,
    layer,
    camera,
    mouse,
    key,
};

enum EventType {
    CREATED,KILLED,UPDATED
};
struct EventId {
    EventId(std::vector<EventIdChunk> hardpath, std::vector<std::string> softpath) : hardpath(hardpath),softpath(softpath) {}
    std::vector<EventIdChunk> hardpath;
    std::vector<std::string> softpath;
    bool is_a(EventIdChunk e);
    bool is(void* object);
};

struct Event {
    Event(EventId p_who, EventType p_what_happened):who(p_who), what_happened(p_what_happened){}
    EventId who;
    EventType what_happened;
};


struct EventManager {
    void push_back(Event event) {
        events.emplace(event);
    }
    void dispatch();
    std::queue<Event> events;
};