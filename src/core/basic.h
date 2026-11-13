#pragma once
#include <ultimaille/all.h>

// just to plop paths for debug 
inline std::ostream& operator<<(std::ostream& os,const std::vector<std::string>& vec){
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i){
        if (i > 0)os << ", ";
        os << vec[i];
    }
    os << "]";
    return os;
}


#define FOR(i, n) for(int i = 0; i < (int) n; i++)
#define plop(x) {std::cerr << "|plop|=>"<<" line:"<< __LINE__ <<"  "<< #x <<" : " <<x<< "     in  "<< __FILE__ <<std::endl;}

struct Log{
    inline static void add(std::string msg0,std::string msg1=""){ std::cerr<<"INFO: "     <<msg0<<" "<<msg1<<std::endl; }
    inline static void error(std::string msg0,std::string msg1=""){ std::cerr<<"ERROR: "    <<msg0<<" "<<msg1<<std::endl; }
    inline static void abort(std::string msg0,std::string msg1=""){ std::cerr<<"FATAL ERROR"<<msg0<<" "<<msg1<<std::endl; }
};

inline const char* label(std::string str, std::string suffix = "") {
    static thread_local std::string s;
    s = (str + "##" + suffix);
    return s.c_str();
};

