//
// Created by Avinash Arjavalingam on 12/5/18.
//

#include <cstdlib>
#include <thread>
#include <climits>
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
    std::mutex data_lock;

    bool is_updating;
    bool stop_updating;
    std::unordered_set<Key> total_set;

    std::unordered_map<Key, int> hot_map;
    std::unordered_map<Key, int> cold_map;

    float threshold_percent;
    int hot_threshold;
    int cold_threshold;

    void set_values(float threshold_percent_arg, int **hash_functions_arg, int l_arg, int B_arg) {
        std::unordered_set<Key> reset_total_set;
        std::unordered_map<Key, int> reset_hot_map;
        std::unordered_map<Key, int> reset_cold_map;

        total_set = reset_total_set;
        hot_map = reset_hot_map;
        cold_map = reset_cold_map;

        hh_sketch = new HeavyHittersSketch(hash_functions_arg, l_arg, B_arg);
        threshold_percent = threshold_percent_arg;

        hot_threshold = 0;
        cold_threshold = INT_MIN;

        is_updating = false;
        stop_updating = false;
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
            if((-1 * kv.second) >= median) {
                new_cold_map[kv.first] = kv.second;
            }
        }

        cold_map = new_cold_map;
        cold_threshold = median;
    };

public:
    AdaptiveThresholdHeavyHitters(float threshold_percent_arg, int **hash_functions_arg, int l_arg, int B_arg) {
        set_values(threshold_percent_arg, hash_functions_arg, l_arg, B_arg);
    };

    int periodic_update() {
        if(!(stop_updating) && !(is_updating)) {
            is_updating = true;
            while(!(stop_updating)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                data_lock.lock();
                int total_size = total_set.size();
                int hot_size = hot_map.size();
                if (hot_size > (threshold_percent * total_size)) {
                    update_hot();
                }

                int cold_size = cold_map.size();
                if (cold_size > (threshold_percent * total_size)) {
                    update_cold();
                }
                data_lock.unlock();
            }
        }

        return 1;
    };

    void stop_checking() {
        data_lock.lock();
        stop_updating = true;
        int total_size = total_set.size();

        int hot_size = hot_map.size();
        if (hot_size > (threshold_percent * total_size)) {
            update_hot();
        }

        int cold_size = cold_map.size();
        if (cold_size > (threshold_percent * total_size)) {
            update_cold();
        }
        data_lock.unlock();
    };

    void report_key(Key key) {
        data_lock.lock();
        total_set.insert(key);

        int new_count = (*hh_sketch).update(key);

        if(new_count > hot_threshold) {
            hot_map[key] = new_count;
        }

        if((-1 * new_count) >= cold_threshold) {
            cold_map[key] = new_count;
        } else {
            if(cold_map.find(key) != cold_map.end()) {
                cold_map.erase(key);
            }
        }
        data_lock.unlock();
    };

    int get_key_count(Key key) {
        int temp_key_count;
        data_lock.lock();
        temp_key_count = (*hh_sketch).estimate(key);
        data_lock.unlock();
        return temp_key_count;
    };

    std::unordered_map<Key, int> get_hot_map(void) {
        std::unordered_map<Key, int> temp_hot_map;
        data_lock.lock();
        for(auto kv: hot_map) {
            temp_hot_map[kv.first] = kv.second;
        }
        data_lock.unlock();
        return temp_hot_map;
    };

    std::unordered_map<Key, int> get_cold_map(void) {
        std::unordered_map<Key, int> temp_cold_map;
        data_lock.lock();
        for(auto kv: cold_map) {
            temp_cold_map[kv.first] = kv.second;
        }
        data_lock.unlock();
        return temp_cold_map;
    };

    int get_hot_threshold() {
        int temp_hot_threshold;
        data_lock.lock();
        temp_hot_threshold = hot_threshold;
        data_lock.unlock();
        return temp_hot_threshold;
    };

    int get_cold_threshold() {
        int temp_cold_threshold;
        data_lock.lock();
        temp_cold_threshold = cold_threshold;
        data_lock.unlock();
        return temp_cold_threshold;
    };

    int get_total_size() {
        int temp_total_set_size;
        data_lock.lock();
        temp_total_set_size = total_set.size();
        data_lock.unlock();
        return temp_total_set_size;
    };

    void reset(float threshold_percent_arg, int **hash_functions_arg, int l_arg, int B_arg) {
        data_lock.lock();
        set_values(threshold_percent_arg, hash_functions_arg, l_arg, B_arg);
        data_lock.unlock();
    };
};

