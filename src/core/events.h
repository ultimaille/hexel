#pragma once
#include <queue>
#include "path.h"

struct Event : EventTypes {
    Path who;
    EventType what_happened;
    Event(const Path& who, EventType what_happened) : who(who), what_happened(what_happened) {}
};

struct EventManager {
    void dispatch();
    std::vector<Event> queue;
};

