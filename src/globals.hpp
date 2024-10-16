#ifndef PMPSOLVER_GLOBALS_HPP
#define PMPSOLVER_GLOBALS_HPP
#define TOLERANCE_OBJ 0.0001
#define DEFAULT_DISTANCE 1000000000
#define DEFAULT_MAX_ITE 1000000000
#define DEFAULT_WEIGHT 0.0
#define DEFAULT_CAPACITY 0.0

#include <vector>
#include <string>


using namespace std;

typedef unsigned int uint_t;
typedef double dist_t;
typedef struct {uint_t node;
                dist_t dist;
                } my_pair;
typedef struct {
    uint_t node;
    dist_t usage;
    dist_t dist;
} my_tuple;
typedef vector<my_tuple> assignment;

extern uint_t LOC_FREQUENCY; // avg. no. of location occurrences in M sub-PMPs (M = LOC_FREQUENCY*N/n)
extern double BW_CUTOFF; // locations within BW_CUTOFF * h from a selected one obtain nonzero voting score
extern double BW_MULTIPLIER; // bandwidth multiplier
extern uint_t LOC_PRIORITY_CNT;
extern uint_t MAX_SUB_P; // maximum value of p allowed in sub-PMPs
extern uint_t FILTERING_SIZE; // when creating the final sub-PMP, max(n, FILTERING_SIZE * p locations) are considered
extern uint_t SUB_PMP_SIZE;
extern uint_t FINAL_PROB_RSSV_SIZE;
extern int THREAD_NUMBER;
extern uint_t CLOCK_START;
extern uint_t CLOCK_LIMIT;  // time expressed in seconds
extern uint_t CLOCK_LIMIT_CPLEX;  // time expressed in seconds
extern uint_t CLOCK_ELAPSED;  // elapsed time since the beginning in seconds
extern bool CLOCK_THREADED;
extern bool VERBOSE;
extern uint_t TOLERANCE_CPT;
extern uint_t K;
extern uint_t PERCENTAGE;
extern uint_t UB_MAX_ITER;


#endif //PMPSOLVER_GLOBALS_HPP