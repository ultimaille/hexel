#include <ultimaille/all.h>
#include "xcf.h"
#include "core.h"

void XCF::kill_multimesh(const std::string& name) {
    plop(size());
    std::map<std::string,MultiMesh>::erase(name);
    plop(size());
    God::events.push_back(Event(ObjectId({xcf}, {name}),EventType::KILLED));
}