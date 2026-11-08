#pragma once
#include <ultimaille/all.h>

using namespace UM;

// Note: temporary struct, waiting for replacement by AttributePath ?
struct AttributeInfo {
    std::string name;
    AttributeBase::TYPE primitive;
    std::string type;
};