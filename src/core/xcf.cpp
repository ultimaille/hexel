#include <ultimaille/all.h>
#include "xcf.h"
#include "core.h"

void XCF::kill_multimesh(const std::string& name) {
    std::map<std::string,MultiMesh>::erase(name);
    God::events.push_back(Event(EventId({xcf}, {}),EventType::KILLED));
}