#ifndef LARGE_PMP_UTILS_HPP
#define LARGE_PMP_UTILS_HPP

#include <chrono>
#include <random>
#include "globals.hpp"
#include <thread>

using namespace std;

chrono::steady_clock::time_point tick();
void tock(chrono::steady_clock::time_point start);
vector<uint_t> getRandomSubvector(vector<uint_t> orig_vector, uint_t new_size, default_random_engine *generator);
<<<<<<< HEAD
bool cmpPair2nd(pair<uint_t, double>& a, pair<uint_t, double>& b);
=======
bool sortbysec(const pair<int,int> &a, const pair<int,int> &b);
bool cmpPair2nd(pair<uint_t, double>& a, pair<uint_t, double>& b);

/**
 * Returns the number of available threads
 */
unsigned int getAvailableThreads(void);

/**
 * Sets the amount of threds used by the program
 *
 * @param number
 */
void setThreadNumber(const int);

/**
 * Sets the clock limit
 *
 * @param limit
 */
void setClockLimit(const uint_t);

/**
 * Checks if the clock limit (CPU time) had been reached at each call. 
 * If the limit had been reached, exits the program.
 */
void checkClock(void);

>>>>>>> origin/main

#endif //LARGE_PMP_UTILS_HPP
