//
// Created by Avinash Arjavalingam on 12/5/18.
//

#include "heavy_hitters.cpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>

class AdaptiveThresholdHeavyHitters {
protected:

    // When the term "report" is used in conjunction with keys, it refers to
    // an action during the lifetime of the KVS node that corresponds to this
    // AdaptiveThresholdHeavyHitters, specifically a key being observed to be
    // used via a write or read operation in the node

    // Heavy hitters or count min sketch used to store the estimated
    // counts of the keys in a space efficient manner
    HeavyHittersSketch* hh_sketch;

    // The percent of keys that can be in each map compared to the total number of unique
    // keys. As an example, a threshold percent of 0.1 means that the capacities of the individual
    // maps can be at most 10 percent of the number of total unique keys in the keyspace.
    static float threshold_percent;

    // The L value is be determined by the class attribute gamma, with L = ceil(log(gamma))
    static float gamma;

    // The B value is be determined by the class attribute epsilon, with B = ceil(e/epsilon)
    static float epsilon;

    // Contains every unique key reported to the sketch
    std::unordered_set<Key> total_set;

    // Contains the keys that have been reported the most
    std::unordered_map<Key, int> hot_map;

    // Contains the keys with the lowest number of times reported to the sketch.
    std::unordered_map<Key, int> cold_map;

    //  The hot threshold is the lowest possible key access count that a key can have to be added to the hot map
    int hot_threshold;

    // The cold threshold is the highest possible key access count to be added to the cold map
    int cold_threshold;

    // The last time the maps have been checked to make sure they have not grown too large in size
    // relative to the threshold percent.
    std::chrono::system_clock::time_point last_update_time;

    // Used in the constructor and the reset function to set the class members, and initialize
    // the heavy hitters sketch
    void set_values() {
        int B_arg = (int)(ceil(exp(1)/epsilon));
        int l_arg = (int)(ceil(log(gamma)));
        int** hash_functions_arg = get_hash_functions(l_arg);

        total_set.clear();
        hot_map.clear();
        cold_map.clear();

        hh_sketch = new HeavyHittersSketch(hash_functions_arg, l_arg, B_arg);

        hot_threshold = 0;
        cold_threshold = INT_MIN;

        last_update_time = std::chrono::system_clock::now();
    };

    // The partition and select functions are the two functions of the
    // quick select algorithm used to determine the median values of
    // the hot and cold maps. These medians are unique to each map.

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

    // Used to "clean" the hot map. The map’s list of access counts (all the
    // values in the key-value pairs) are collected, and the cleaning function
    // finds the median of these mapped values. Then all values below the median
    // are removed from the map
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

    // Used to "clean" the cold map. The map’s list of access counts (all the
    // values in the key-value pairs) are collected, and the cleaning function
    // finds the median of these mapped values. Then all values above the median
    // are removed from the map
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

    // Used to create the hash functions which are then used by heavy_hitters.cpp
    // These hash functions consist of a coefficient and a constant
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
    AdaptiveThresholdHeavyHitters() {
        set_values();
    };

    // When a key is reported, it is added to the total_set if unique, and then
    // to the heavy hitters sketch, which returns the sketch's estimated value.
    // Then, it sees if the estimated key count would allow it entrance to
    // either the hot or cold map, and adds it to the relevant map if so.
    // Finally, the function checks if at least 10 milliseconds have passed.
    // If so, it checks both maps, and sees if the maps contain fewer than
    // (threshold percent * total number of unique keys) keys. For the maps that
    // violate this condition, the cleaning function (update_hot/cold) is run.
    void report_key(Key key) {
        total_set.insert(key);

        int new_count = (*hh_sketch).update(key);

        if(new_count > hot_threshold) {
            hot_map[key] = new_count;
        }

        if((-1 * new_count) >= cold_threshold) {
            cold_map[key] = new_count;
        } else {
            if(cold_map.find(key) != cold_map.end()) {
                cold_map[key] = new_count;
            }
        }

        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> passed = now - last_update_time;
        double passed_num = passed.count();

        if(passed_num > 10) {
            int total_size = total_set.size();
            int hot_size = hot_map.size();
            if (hot_size > (threshold_percent * total_size)) {
                update_hot();
            }

            int cold_size = cold_map.size();
            if (cold_size > (threshold_percent * total_size)) {
                update_cold();
            }
            last_update_time = now;
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

    int get_hot_threshold() {
        return hot_threshold;
    };

    int get_cold_threshold() {
        return cold_threshold;
    };

    int get_total_size() {
        return total_set.size();
    };

    void reset() {
        set_values();
    };

};

float AdaptiveThresholdHeavyHitters::threshold_percent = 0.01;
float AdaptiveThresholdHeavyHitters::gamma = 4127;
float AdaptiveThresholdHeavyHitters::epsilon = 0.001;

