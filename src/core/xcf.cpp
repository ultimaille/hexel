#include <ultimaille/all.h>
#include "xcf.h"
#include "core.h"

void XCF::erase(const std::string& name) {
    std::map<std::string,MultiMesh>::erase(name);
    God::events.push_back({Event::MM_REMOVED,name});
}