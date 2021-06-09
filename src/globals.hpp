#ifndef LARGE_PMP_GLOBALS_HPP
#define LARGE_PMP_GLOBALS_HPP

#define TOLERANCE 0.000001
#define DEFAULT_DISTANCE 1000000000
#define DEFAULT_WEIGHT 0

using namespace std;

typedef unsigned int uint_t;
typedef double dist_t;
typedef struct {uint_t node;
                dist_t dist;
                } my_pair;

extern uint_t LOC_FREQUENCY; // avg. no. of location occurrences in M sub-PMPs (M = LOC_FREQUENCY*N/n)
extern double BW_CUTOFF; // locations within BW_CUTOFF * h from a selected one obtain nonzero voting score
extern uint_t LOC_PRIORITY_CNT;
extern uint_t MAX_SUB_P; // maximum value of p allowed in sub-PMPs
extern uint_t FILTERING_SIZE; // when creating the final sub-PMP, max(n, FILTERING_SIZE * p locations) are considered
extern uint_t SUB_PMP_SIZE;

#endif //LARGE_PMP_GLOBALS_HPP
