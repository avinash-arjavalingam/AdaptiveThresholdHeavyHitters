//
// Created by Avinash Arjavalingam on 12/5/18.
//

#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cmath>
#include <stdlib.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

typedef std::string Key;

const long large_prime = 4294967311l;

class HeavyHittersSketch {
protected:
    unsigned l, B;

    int **hash_functions;
    int **sketch_array;

    unsigned long hash_key(Key key) {
        unsigned long hash = 5381;
        int char_int = 0;
        for(unsigned i = 0; i < key.length(); i++) {
            char_int = (int)(key.at(i));
            hash = ((hash << 5) + hash) + char_int;
        }

        return hash;
    };

    unsigned location(int i, unsigned long hash) {
        return (unsigned)(((long)hash_functions[i][0]*hash+hash_functions[i][1])%large_prime%B);
    };

    void set_values(int **hash_functions_arg, int l_arg, int B_arg) {
        hash_functions = hash_functions_arg;
        l = l_arg;
        B = B_arg;

        sketch_array = new int *[l];
        for(unsigned i = 0; i < l; i++) {
            sketch_array[i] = new int[B];
            for(unsigned j = 0; j < B; j++) {
                sketch_array[i][j] = 0;
            }
        }
    };

public:
    HeavyHittersSketch(int **hash_functions_arg, int l_arg, int B_arg) {
        set_values(hash_functions_arg, l_arg, B_arg);
    };

    int update(Key key) {
        unsigned long hash = hash_key(key);

        int mincount = 0;
        unsigned hashed_location = 0;
        for (unsigned i = 0; i < l; i++) {
            hashed_location = location(i, hash);
            sketch_array[i][hashed_location] = sketch_array[i][hashed_location] + 1;
            if((sketch_array[i][hashed_location] < mincount) or (i == 0)) {
                mincount = sketch_array[i][hashed_location];
            }
        }

        return mincount;
    };

    int estimate(Key key) {
        unsigned long hash = hash_key(key);

        int mincount = 0;
        unsigned hashed_location = 0;
        for (unsigned i = 0; i < l; i++) {
            hashed_location = location(i, hash);
            if((sketch_array[i][hashed_location] < mincount) or (i == 0)) {
                mincount = sketch_array[i][hashed_location];
            }
        }

        return mincount;
    };

    void reset(int **hash_functions_arg, int l_arg, int B_arg) {
        set_values(hash_functions_arg, l_arg, B_arg);
    };
};