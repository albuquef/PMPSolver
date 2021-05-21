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


#endif //LARGE_PMP_UTILS_HPP
