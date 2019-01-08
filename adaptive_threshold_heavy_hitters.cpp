//
// Created by Avinash Arjavalingam on 12/5/18.
//

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>
#include <stdlib.h>
#include <stdbool.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "heavy_hitters.cpp"

typedef std::string Key;

class AdaptiveThresholdHeavyHitters {
protected:
    HeavyHittersSketch* hh_sketch;

    int total_hits;
    std::unordered_set<Key> total_set;

    std::unordered_map<Key, int> hot_map;
    std::unordered_map<Key, int> cold_map;

    float threshold_percent;
    int hot_threshold;
    int cold_threshold;

    int min_threshold;
    bool threshold_met;

    void set_values(float threshold_percent_arg, int **hash_functions_arg, int l_arg, int B_arg) {
        std::unordered_set<Key> reset_total_set;
        std::unordered_map<Key, int> reset_hot_map;
        std::unordered_map<Key, int> reset_cold_map;

        total_set = reset_total_set;
        hot_map = reset_hot_map;
        cold_map = reset_cold_map;

        hh_sketch = new HeavyHittersSketch(hash_functions_arg, l_arg, B_arg);
        threshold_percent = threshold_percent_arg;

        total_hits = 0;

        hot_threshold = 0;
        cold_threshold = 0;

        min_threshold = 100;
        threshold_met = false;
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

    void update_hot(void) {
        int* vals;
        std::unordered_map<Key, int> new_hot_map;
        std::vector<int> val_vec;
        int val_size = 0;

        for(auto kv: hot_map) {
            val_size = val_size + 1;
            val_vec.push_back(kv.second);
        }

        vals = (int*)(&val_vec[0]);

        int median = select(vals, 0, val_size - 1, 1);

        for(auto kv: hot_map) {
            if(kv.second > median) {
                new_hot_map[kv.first] = kv.second;
            }
        }

        hot_map = new_hot_map;
        hot_threshold = median;
    };

    /*
    void update_cold(void) {
        int* vals;
        std::unordered_map<Key, int> new_cold_map;
        std::vector<int> val_vec;
        int val_size = 0;

        for(auto kv: cold_map) {
            val_size = val_size + 1;
            val_vec.push_back(kv.second);
        }

        vals = &val_vec[0];

        int median = (-1 * select(vals, 0, val_size - 1, 1));

        for(auto kv: cold_map) {
            if((-1 * kv.second) > median) {
                new_cold_map[kv.first] = kv.second;
            }
        }

        cold_map = new_cold_map;
        cold_threshold = median;
    };
    */

public:
    AdaptiveThresholdHeavyHitters(float threshold_percent_arg, int **hash_functions_arg, int l_arg, int B_arg) {
        set_values(threshold_percent_arg, hash_functions_arg, l_arg, B_arg);
    };

    void report_key(Key key) {
        total_set.insert(key);
        total_hits = total_hits + 1;

        int new_count = (*hh_sketch).update(key);

        if(new_count > hot_threshold) {
            /*
            if(hot_map.find(key) != hot_map.end()) {
                hot_map[key] = new_count;
            } else {
                hot_map[key] = new_count;
            }
            */
            hot_map[key] = new_count;
        }

        /*
        if((-1 * new_count) >= cold_threshold) {
            if(cold_map.find(key) != cold_map.end()) {
                cold_map[key] = new_count;
            } else {
                cold_map[key] = new_count;
            }
        }
        */

        cold_map[key] = new_count;

        int total_size = total_set.size();

        /*
        if(!(threshold_met) && (total_size > min_threshold)) {
            threshold_met = true;
            update_hot();
            update_cold();
        }

        if(threshold_met) {
            int hot_size = hot_map.size();
            if (hot_size > (threshold_percent * total_size)) {
                update_hot();
            }

            int cold_size = cold_map.size();
            if (cold_size > (threshold_percent * total_size)) {
                update_cold();
            }
        }
        */

        int hot_size = hot_map.size();
        if (hot_size > (threshold_percent * total_size)) {
            update_hot();
        }

    };

    int get_key_count(Key key) {
        return (*hh_sketch).estimate(key);
    };

    std::unordered_map<Key, int> get_hot_map(void) {
        return hot_map;
    };

    std::unordered_map<Key, int> get_cold_map(void) {
        return cold_map;
    };

    double get_average(void) {
        int total_size = total_set.size();
        double avg = total_hits / ((double)(total_size));
        return avg;
    };

    int get_hot_threshold() {
        return hot_threshold;
    };

    void reset(float threshold_percent_arg, int **hash_functions_arg, int l_arg, int B_arg) {
        set_values(threshold_percent_arg, hash_functions_arg, l_arg, B_arg);
    };
};

