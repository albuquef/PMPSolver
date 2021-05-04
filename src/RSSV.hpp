#ifndef LARGE_PMP_RSSV_HPP
#define LARGE_PMP_RSSV_HPP

#include <unordered_map>
#include "instance.hpp"
#include "solution.hpp"
#include "semaphore.hpp"


using namespace std;

class RSSV {
private:
    Instance *instance;
    default_random_engine engine;
    uint_t N;
    uint_t M;
    uint_t n;
    Semaphore sem;
    mutex weights_mutex;
    unordered_map<uint_t, double> weights;
public:
    RSSV(Instance *instance, uint_t seed, uint_t n);
    Solution run();
    void solveSubproblem(uint_t seed);
    void processSubsolution(Solution *solution);
};


#endif //LARGE_PMP_RSSV_HPP
