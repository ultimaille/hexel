#pragma once
#include "layers/basic_layers.h"
#include "core/event.h"

// Horrible global variables
extern ObjectId grad_and_drop_objectid;

bool FilePopup(const char* id, std::string in, std::string& out, std::vector<const char*> extensions);
using namespace events;
void look_at_pointset(PointSet& ps);
void load_mm_with_default_layers(std::string path);
