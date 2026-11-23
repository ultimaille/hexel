#pragma once
#include <ultimaille/all.h>
using namespace UM;

#define FOR(i, n) for(int i = 0; i < (int) n; i++)
#define plop(x) {std::cerr << "|plop|=>"<<" line:"<< __LINE__ <<"  "<< #x <<" : " <<x<< "     in  "<< __FILE__ <<std::endl;}

#include "log.h"


inline const char* label(std::string str, std::string suffix = "") {
    static thread_local std::string s;
    s = (str + "##" + suffix);
    return s.c_str();
};

