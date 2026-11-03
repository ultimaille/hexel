#pragma once
#include <algorithm>
#include <random>
#include <set>
#include <iostream>
#include <chrono>
#include <numeric>
struct Reorder{
    inline Reorder(int n) :old2new(n), neig(n){
        start = std::chrono::system_clock::now();
        std::iota(old2new.begin(), old2new.end(), 0);
    }

    inline int duration() {
        return  std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now() - start).count();
    };

    inline void link(int a, int b) {
        neig[a].emplace(b);
        neig[b].emplace(a);
    }

    inline int dist(int i) {
        int res = 1000000;
        for (int j : neig[i]) res = std::min(res, std::abs(old2new[i] - old2new[j]));
        return res;
    }

    inline int optim_pass(int min_dist) {
        int nfails = 0;
        int nretries = 1000;
        for (int i = old2new.size() - 1; i > 0; i--) {
            if (dist(i) < min_dist) for (int retry = 0; retry < nretries;retry++){
                int j = rand() % i;
                std::swap(old2new[i], old2new[j]);
                if (dist(i) < min_dist || dist(j) < min_dist){
                    if (dist(i) < min_dist)
                        std::swap(old2new[i], old2new[j]);
                }
                else break;
                if (retry == nretries - 1) nfails++;
            }
        }
        return nfails;
    }

    inline void init() {
        auto rng = std::default_random_engine{};
        std::shuffle(std::begin(old2new), std::end(old2new), rng);
    }
    inline void apply(int allowed_milliseconds = 100,bool verbose = false) {
        init();
        if(verbose) std::cerr << "Init duration= " << duration() << " milliseconds\n";
        int frac = 20;
        while (frac > 1 && allowed_milliseconds > duration())
            if (optim_pass(old2new.size() / frac) == 0) 
                frac--;
        
        if (verbose) std::cerr << "Shuffle done in " << duration() << " milliseconds " << " for minimal distance of 1/" << frac << " in the color map\n";
    }

    std::chrono::time_point<std::chrono::system_clock> start;
    std::vector<int> old2new;
    std::vector<std::set<int>> neig;
};


#include <ultimaille/all.h>
#include <map>
using namespace UM;

inline void smart_shuffle_edge_attribute(PolyLine& pl,EdgeAttribute<int>& value, int allowed_computation_time_in_milliseconds=100){
    um_assert(pl.connected());
    std::map<int, int> compress_map;
    int compressed_size = 0;
    for (auto e : pl.iter_edges()) if (!compress_map.contains(value[e])) compress_map[value[e]] = compressed_size++;
    for (auto e : pl.iter_edges()) value[e] = compress_map[value[e]];

    Reorder reo(compressed_size);
    for (auto v : pl.iter_vertices()) 
        for (auto e0 : v.iter_edges())for (auto e1 : v.iter_edges())
            if(value[e0]!=value[e1]) reo.link(value[e0], value[e1]);
    reo.apply(allowed_computation_time_in_milliseconds,true);
    for (auto e : pl.iter_edges()) value[e] = reo.old2new[value[e]];
}

