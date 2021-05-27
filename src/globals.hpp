#ifndef LARGE_PMP_GLOBALS_HPP
#define LARGE_PMP_GLOBALS_HPP

#include <vector>

#define TOLERANCE 0.000001
#define DEFAULT_DISTANCE 1000000000
#define DEFAULT_WEIGHT 0
#define DEFAULT_CAPACITY 0

using namespace std;

typedef unsigned int uint_t;
typedef double dist_t;
typedef struct {uint_t node;
                dist_t dist;
                } my_pair;
typedef struct {
    uint_t node;
    uint_t usage;
    dist_t dist;
} my_tuple;
typedef vector<my_tuple> assignment;

extern uint_t LOC_FREQUENCY; // avg. no. of location occurrences in M sub-PMPs (M = LOC_FREQUENCY*N/n)
extern double BW_CUTOFF; // locations within BW_CUTOFF * h from a selected one obtain nonzero voting score

#endif //LARGE_PMP_GLOBALS_HPP
