#include "globals.hpp"

double BW_CUTOFF = 4;
uint_t LOC_PRIORITY_CNT = 15;
uint_t MAX_SUB_P = 100;
uint_t FILTERING_SIZE = 4;
uint_t LOC_FREQUENCY = 5; // how many times each location is used in subproblems
uint_t SUB_PMP_SIZE = 800; // size of subproblems for RSSV
int THREAD_NUMBER = 1;
uint_t CLOCK_START = 0;
uint_t CLOCK_LIMIT = 3600;
uint_t CLOCK_ELAPSED = 0;
bool CLOCK_THREADED = false;
bool VERBOSE = false;
uint_t TOLERANCE_CPT = 10;
uint_t K = 50;
uint_t PERCENTAGE = 50;