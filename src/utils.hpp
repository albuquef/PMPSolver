#ifndef LARGE_PMP_UTILS_HPP
#define LARGE_PMP_UTILS_HPP

#include <chrono>
#include <random>
#include "types.hpp"

using namespace std;

chrono::steady_clock::time_point tick();
void tock(chrono::steady_clock::time_point start);
vector<uint_t> sampleSubvector(vector<uint_t> *orig_vector, uint_t max_value, uint_t new_size, default_random_engine *generator);

#endif //LARGE_PMP_UTILS_HPP
