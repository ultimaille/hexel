#pragma once
#include <ultimaille/all.h>

#define FOR(i, n) for(int i = 0; i < (int) n; i++)
#define plop(x) {std::cerr << "|plop|=>"<<" line:"<< __LINE__ <<"  "<< #x <<" : " <<x<< "     in  "<< __FILE__ <<std::endl;}

struct Log{
    inline static void add(std::string msg0,std::string msg1=""){ std::cerr<<"INFO: "     <<msg0<<" "<<msg1<<std::endl; }
    inline static void error(std::string msg0,std::string msg1=""){ std::cerr<<"ERROR: "    <<msg0<<" "<<msg1<<std::endl; }
    inline static void abort(std::string msg0,std::string msg1=""){ std::cerr<<"FATAL ERROR"<<msg0<<" "<<msg1<<std::endl; }
};

// Registry<T> is a lightweight named container for polymorphic objects (=> all stored types must inherit from T).
//
// The registry owns instances through std::unique_ptr<T>, so callers can
// manipulate them through the base interface without dealing with ownership.
// Each object is associated with a unique string name, which makes lookup by
// name convenient while preserving insertion order for iteration.

template<class T>
struct Registry {
    struct Element {
        std::string name;
        std::unique_ptr<T> object;
    };
    std::vector<Element> items;

    template<class P, class... Args>
        P& emplace_back(std::string name, Args&&... args) {
            static_assert(std::is_base_of_v<T, P>);
            um_assert(!contains(name));

            auto object = std::make_unique<P>(
                    std::forward<Args>(args)...
                    );

            items.push_back({
                    std::move(name),
                    std::move(object)
                    });

            return static_cast<P&>(*items.back().object);
        }

    template<class... Args>
        T& emplace_back(std::string name, Args&&... args) {
            return emplace_back<T>(
                    std::move(name),
                    std::forward<Args>(args)...
                    );
        }

    bool contains(const std::string& name) const {
        return find(name) != -1;
    }

    int find(const std::string& name) const {
        for (int i = 0; i < size(); ++i)
            if (items[i].name == name)
                return i;
        return -1;
    }

    T& operator[](int index) {
        um_assert(index >= 0 && index < size());
        return *items[index].object;
    }

    const T& operator[](int index) const {
        um_assert(index >= 0 && index < size());
        return *items[index].object;
    }

    T& operator[](const std::string& name) {
        int index = find(name);
        um_assert(index >= 0 && index < size());
        return *items[index].object;
    }

    const T& operator[](const std::string& name) const {
        int index = find(name);
        um_assert(index >= 0 && index < size());
        return *items[index].object;
    }

    void pop_back() {
        um_assert(!empty());
        items.pop_back();
    }

    int size() const {
        return static_cast<int>(items.size());
    }

    void erase(int index) {
        um_assert(index >= 0 && index < size());
        items.erase(begin() + index);
    }

    void erase(std::string &name) {
        int index = find(name);
        um_assert(index >= 0 && index < size());
        erase(index);
    }

    bool empty() const {
        return items.empty();
    }

    auto begin() {
        return items.begin();
    }

    auto end() {
        return items.end();
    }

    auto begin() const {
        return items.cbegin();
    }

    auto end() const {
        return items.cend();
    }
};

