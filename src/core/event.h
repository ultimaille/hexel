#pragma once
#include <queue>
#include "objectid.h"

using namespace UM;

struct Event {
    ObjectId who;
    events::EventType what_happened;
};

struct EventManager {
    void dispatch();
    std::queue<Event> queue;
};                                                           

