//
// Created by Avinash Arjavalingam on 12/5/18.
//

#include <vector>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <stdlib.h>
#include "adaptive_threshold_heavy_hitters.cpp"
#include "statsHelper.hpp"

#define gamma 0.01
#define epsilon 0.00025

typedef std::string Key;

int** get_hash_functions(int l) {
    int **hash_functions;
    srand(time(NULL));
    hash_functions = new int *[l];
    for(unsigned i = 0; i < l; i++) {
        hash_functions[i] = new int[2];
        hash_functions[i][0] = int(float(rand())*float(large_prime)/float(RAND_MAX) + 1);
        hash_functions[i][1] = int(float(rand())*float(large_prime)/float(RAND_MAX) + 1);
    }
    return hash_functions;
}

int get_l(void) {
    int l = (int)(ceil(log(1/gamma)));
    return l;
}

int get_B(void) {
    int B = (int)(ceil(exp(1)/epsilon));
    return B;
}

int main() {
    double count = 1;
    double totalDev = 0;
    double avgErr = 0;
    int i = 0;
    int l = get_l();
    int B = get_B();
    std::cout << "L: " << l << std::endl;
    std::cout << "B: " << B << std::endl;
    int** hash_functions = get_hash_functions(l);
    Key key1 = "testing";
    std::vector<Key> inputs = generateZipWorkload(10000, 0.8, 0);
    std::unordered_map<std::string, int> freqMap = computeFrequencies(inputs);
    std::pair<double, double> stats = computeStats(freqMap);
    HeavyHittersSketch *hh = new HeavyHittersSketch(hash_functions, l, B);
    for (auto k: inputs) {
        hh->update(k);
    }
    std::cout << "Mean: " << stats.first << std::endl;
    std::cout << "Standard Deviation: " << stats.second << std::endl;
    for (auto k: inputs) {
        // std::cout << k << " standard key: " << freqMap[k] << std::endl;
        // std::cout << k << " heavyHitters: " << hh->estimate(k) << std::endl;
        count += 1;
        totalDev += ((hh->estimate(k)) - (freqMap[k])) / (freqMap[k]);
    }
    avgErr = totalDev / count;
    std::cout << "Average Error: " << avgErr << std::endl;




    AdaptiveThresholdHeavyHitters *athh = new AdaptiveThresholdHeavyHitters(0.01, hash_functions, l, B);
    for (auto k: inputs) {
        athh->report_key(k);
    }

    std::cout<< "ATHH count: " << athh->get_average() << std::endl;
    /*
    for (auto k: inputs) {
        std::cout << k << " standard key:   " << freqMap[k] << std::endl;
        std::cout << k << " ATHH key count: " << athh->get_key_count(k) << std::endl;
    }
    */
    std::cout << "Total count: " << (athh->get_total_size()) << std::endl;
    std::cout << "Hot count: " << (athh->get_hot_map()).size() << std::endl;

    for(auto kv: athh->get_hot_map()) {
        std::cout << kv.first << ": " << kv.second << std::endl;
    }

    std::cout << "Hot threshold: " << athh->get_hot_threshold()<< std::endl;
    std::cout << "Total hot update: " << athh->get_thu()<< std::endl;



    std::cout << "Cold count: " << (athh->get_cold_map()).size() << std::endl;

    for(auto kv: athh->get_cold_map()) {
        std::cout << kv.first << ": " << kv.second << std::endl;
    }

    std::cout << "Cold threshold: " << athh->get_cold_threshold()<< std::endl;
    std::cout << "Total cold update: " << athh->get_tcu()<< std::endl;
}