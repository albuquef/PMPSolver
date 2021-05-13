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

#endif //LARGE_PMP_GLOBALS_HPP
