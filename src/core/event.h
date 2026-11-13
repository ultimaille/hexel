#pragma once
#include <queue>
#include "objectid.h"

using namespace UM;

struct Event {
    Path who;
    events::EventType what_happened;
};

struct EventManager {
    void dispatch();
    std::vector<Event> queue;
};

