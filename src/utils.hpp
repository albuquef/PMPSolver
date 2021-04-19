#ifndef LARGE_PMP_UTILS_HPP
#define LARGE_PMP_UTILS_HPP

#include <chrono>

using namespace std;

chrono::steady_clock::time_point tick();
void tock(chrono::steady_clock::time_point start);

#endif //LARGE_PMP_UTILS_HPP
