//
// Created by Avinash Arjavalingam on 2/27/19.
//

#include <cstdlib>
#include <thread>
#include <climits>
#include <iostream>
#include <ctime>
#include <cmath>
#include <utility>
#include <stdlib.h>
#include <stdbool.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iomanip>
#include <chrono>
#include <stdio.h>

#define alpha 0.2

typedef std::string Key;

class RegularHeavyHitters {
protected:
    HeavyHittersSketch* hh_sketch;
    static float threshold_percent;
    static float gamma;
    static float epsilon;

    std::unordered_set<Key> total_set;

    void set_values() {
        int B_arg = (int)(ceil(exp(1)/epsilon));
        int l_arg = (int)(ceil(log(gamma)));
        int** hash_functions_arg = get_hash_functions(l_arg);
        std::unordered_set<Key> reset_total_set;

        total_set = reset_total_set;

        hh_sketch = new HeavyHittersSketch(hash_functions_arg, l_arg, B_arg);
    };

    int partition(int list[], int left, int right, int pivotIndex) {
        int pivotValue = list[pivotIndex];

        int tmp = list[pivotIndex];
        list[pivotIndex] = list[right];
        list[right] = tmp;

        int storeIndex = left;
        for (int i = left; i < right - 1; i++)
        {
            if (list[i] < pivotValue)
            {
                tmp = list[storeIndex];
                list[storeIndex] = list[i];
                list[i] = list[storeIndex];

                storeIndex++;
            }
        }

        tmp = list[right];
        list[right] = list[storeIndex];
        list[storeIndex] = list[right];
        return storeIndex;
    };

    int select(int list[], int left, int right, int k) {
        if (left == right)
        {
            return list[left];
        }

        int pivotIndex = right;

        pivotIndex = partition(list, left, right, pivotIndex);

        if (k == pivotIndex)
        {
            return list[k];
        }
        else if (k < pivotIndex)
        {
            return select(list, left, pivotIndex - 1, k);
        }
        else
        {
            return select(list, pivotIndex + 1, right, k);
        }
    };

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
    };

public:
    RegularHeavyHitters() {
        set_values();
    };

    void report_key(Key key) {
        total_set.insert(key);
        int new_count = (*hh_sketch).update(key);
    };

    int get_key_count(Key key) {
        return (*hh_sketch).estimate(key);
    };

    int get_total_size() {
        return total_set.size();
    };

    std::unordered_map<Key, int> get_all_map(void) {
        std::unordered_map<Key, int> all_map;

        for(auto new_key: total_set) {
            all_map[new_key] = this->get_key_count(new_key);
        }

        return all_map;
    };
    
    void reset() {
        set_values();
    };

    static void reset_threshold_percent(float new_threshold) {
        threshold_percent = new_threshold;
    };

    static void reset_error(float new_epsilon) {
        epsilon = new_epsilon;
    };

    static void update_gamma(int total_hits, int num_hh) {
        float avg_hit = (1.0 * total_hits) / num_hh;
        gamma = (alpha * avg_hit) + ((1 - alpha) * gamma);
    };

};

float RegularHeavyHitters::threshold_percent = 0.1;
float RegularHeavyHitters::gamma = 4127;
float RegularHeavyHitters::epsilon = 0.001;