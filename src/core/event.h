#pragma once
#include <vector>
#include <string>

// -------------------------------------------------------------------------------
//                                    Events
// -------------------------------------------------------------------------------
// we need some way to have objects interactions. Messages, listeners and callback would not simplify the conception
struct Event{
    enum {
        MOUSE_MOVED,
        MOUSE_PRESSED,
        MOUSE_RELEASED,
        KEY_PRESSED,
        KEY_RELEASED,
        MM_REMOVED,
        TRIANGLES_REMOVED,
        TRIANGLES_UPDATED
    } even_type;

    std::string object_name;// a string that allows to find the element
};

struct EventManager {

    void push_back(Event event) {
        events.push_back(event);
    }

    void dispatch();

    void clear() {
        events.clear();
    }

    std::vector<Event> events;

};