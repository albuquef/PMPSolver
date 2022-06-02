#ifndef LARGE_PMP_UTILS_HPP
#define LARGE_PMP_UTILS_HPP

#include <chrono>
#include <random>
#include "globals.hpp"

using namespace std;

chrono::steady_clock::time_point tick();
void tock(chrono::steady_clock::time_point start);
vector<uint_t> getRandomSubvector(vector<uint_t> orig_vector, uint_t new_size, default_random_engine *generator);
bool sortbysec(const pair<int,int> &a, const pair<int,int> &b);
bool cmpPair2nd(pair<uint_t, double>& a, pair<uint_t, double>& b);

/**
 * Sets the clock limit
 *
 * @param limit
 */
void setClockLimit(uint_t);

/**
 * Checks if the clock limit (CPU time) had been reached at each call. 
 * If the limit had been reached, exits the program.
 */
void checkClock(void);


#endif //LARGE_PMP_UTILS_HPP
