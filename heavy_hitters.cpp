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

// Used for the creation of the universal hash functions
// The large prime is used to create random integer values for the
// hash functions.
const long large_prime = 4294967311l;

// Heavy hitters sketch largely sourced from Daniel Alabi from Github
// Daniel Alabi's github profile: https://github.com/alabid
// Source code: https://github.com/alabid/countminsketch

class HeavyHittersSketch {
protected:

    // For a greater detail of explanation of the concept of a heavy hitters
    // or count min sketch that will be presented in the comments visit:
    // http://inst.eecs.berkeley.edu/~cs170/fa16/lecture-11-22.pdf

    // l defines the number of hash functions that the sketch has,
    // with more hash functions meaning greater accuracy in estimated values
    // but higher space requirements.
    unsigned l;


    // B defines the range of the hash functions, and the size of the arrays
    // that store the estimated values. Like l, a larger B means greater accuracy
    // but larger space usage.
    unsigned B;

    // Hash functions is a 2-D array with dimensions l by 2, with l hash functions
    // and 2 integer values per hash function, one coefficient and one constant, to
    // map the key's onto the l sketch arrays
    int **hash_functions;

    // An l by B integer array. Each B length array has a corresponding hash functions
    // (discussed above). After applying the hash function to the key, the value from that
    // operation is taken modulo B, in order to map it onto the range of each array.
    int **sketch_array;

    // Takes a Key-type key and translates it into a integer, by taking the integer
    // representation of each character in the string, and applying the hashing function.
    // It is important to note that his hash function is separate and unrelated to the hash
    // function that determines the locations of each key in the sketch
    unsigned long hash_key(Key key) {
        unsigned long hash = 5381;
        int char_int = 0;
        for(unsigned i = 0; i < key.length(); i++) {
            char_int = (int)(key.at(i));
            hash = ((hash << 5) + hash) + char_int;
        }

        return hash;
    };

    // Translates the integer representation of a key into a location on the hh sketch.
    // The first parameter, integer i, determines which of the l arrays is being used,
    // and applies that array's hash function to the second parameter, the key int
    // representation.
    unsigned location(int i, unsigned long hash) {
        return (unsigned)(((long)hash_functions[i][0]*hash+hash_functions[i][1])%large_prime%B);
    };

    // Initializes a new sketch of size l by B. It sets the sketch's members, and initializes
    // all the values in the sketch_array to 0, to later be incremented as the program is run
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
    // Constructor, parameters are explained above.
    HeavyHittersSketch(int **hash_functions_arg, int l_arg, int B_arg) {
        set_values(hash_functions_arg, l_arg, B_arg);
    };

    // When a key is reported to the heavy hitters sketch, update is called on the key
    // to record the incident in the sketch. As an overview, for all arrays of the sketch,
    // the key is translated to a long, which itself is passed through location (explained above),
    // and then the integer at the index corresponding the returned value of location is incremented.
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

    // Looks up the estimated value of a key. As an overview, it iterates through all
    // the hashed locations of the key at all l arrays, and then chooses the lowest value.
    // For an explanation on why this works, use the pdf referenced above.
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

    // Resets all the values in the sketch to 0, erasing any previous estimated key counts
    void reset(int **hash_functions_arg, int l_arg, int B_arg) {
        set_values(hash_functions_arg, l_arg, B_arg);
    };
};