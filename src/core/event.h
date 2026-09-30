#pragma once
#include <vector>
#include <string>
#include <queue>
#include <iostream>


extern const std::string chunk_xcf;
extern const std::string chunk_pointset;
extern const std::string chunk_polylines;
extern const std::string chunk_triangles;
extern const std::string chunk_quads;
extern const std::string chunk_polygons;
extern const std::string chunk_tetrahedra;
extern const std::string chunk_hexahedra;
extern const std::string chunk_wedges;
extern const std::string chunk_pyramids;
extern const std::string chunk_layer;
extern const std::string chunk_camera;
extern const std::string chunk_mouse;
extern const std::string chunk_key;



enum EventType {
    CREATED, KILLED, UPDATED
};

struct ObjectId {
    ObjectId(std::vector<std::string> p_chunks = {}) : chunks(p_chunks) {}
    std::vector<std::string> chunks;
    void* ptr();
    void emit(EventType e);
    inline void show() {
        for (auto c : chunks) std::cerr << " ==> " << c;
        std::cerr << std::endl;
    }
};

inline bool operator==(const ObjectId& a, const  ObjectId& b) {
    return a.chunks == b.chunks;
}

struct Event {
//    Event(ObjectId p_who, EventType p_what_happened) : who(p_who), what_happened(p_what_happened) {}
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
