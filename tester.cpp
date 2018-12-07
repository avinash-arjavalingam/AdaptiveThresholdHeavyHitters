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

#define gamma 0.01
#define epsilon 0.05

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
    int l = get_l();
    int B = get_B();
    int** hash_functions = get_hash_functions(l);
    AdaptiveThresholdHeavyHitters *athh = new AdaptiveThresholdHeavyHitters(0.2, hash_functions, l, B);
}