//
// Created by Avinash Arjavalingam on 12/5/18.
//

#include <chrono>
#include <thread>
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
#include <ctime>
#include <iomanip>

typedef std::string Key;

int main() {
    std::vector<Key> inputs = generateZipWorkload(10000, 0.8, 0);
    std::unordered_map<std::string, int> freqMap = computeFrequencies(inputs);
    std::pair<double, double> stats = computeStats(freqMap);

    AdaptiveThresholdHeavyHitters *athh = new AdaptiveThresholdHeavyHitters();

    for (auto k: inputs) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        athh->report_key(k);
    }

    std::cout << "Total count: " << (athh->get_total_size()) << std::endl;
    std::cout << "Hot count: " << (athh->get_hot_map()).size() << std::endl;

    for(auto kv: athh->get_hot_map()) {
        std::cout << kv.first << ": " << kv.second << std::endl;
    }

    std::cout << "Hot threshold: " << athh->get_hot_threshold()<< std::endl;


    std::cout << "Cold count: " << (athh->get_cold_map()).size() << std::endl;

    for(auto kv: athh->get_cold_map()) {
        std::cout << kv.first << ": " << kv.second << std::endl;
    }

    std::cout << "Cold threshold: " << athh->get_cold_threshold()<< std::endl;
}