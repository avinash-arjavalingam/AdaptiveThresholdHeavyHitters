//
// Created by Avinash Arjavalingam on 12/5/18.
//

#include <iostream>
#include <fstream>
#include "adaptive_threshold_heavy_hitters.cpp"
#include "statsHelper.hpp"
#include <thread>

typedef std::string Key;

int main() {

    std::vector<Key> inputs;

    std::ifstream in("cli.txt");

    if(!in) {
        std::cout << "Cannot open input file.\n";
        return 1;
    }

    char strLine[255];

    while(in) {
        in.getline(strLine, 255);  // delim defaults to '\n'
        if(in) {
            std::string line = std::string(strLine);
            Key inp = line.substr(4, 8);
            inputs.push_back(inp);
            // std::cout << line.substr(4, 8) << std::endl;
        }

    }

    in.close();


    // std::vector<Key> inputs = generateZipWorkload(10000, 0.8, 0);

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