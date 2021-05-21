#ifndef LARGE_PMP_RSSV_HPP
#define LARGE_PMP_RSSV_HPP

#include <unordered_map>
#include <thread>
#include "TB.hpp"
#include "instance.hpp"
#include "instance.hpp"
#include "solution.hpp"
#include "semaphore.hpp"

using namespace std;

class RSSV {
private:
    shared_ptr<Instance> instance; // original PMP instance
    default_random_engine engine;
    uint_t N; // original PMP size (no. of locations)
    uint_t M; // no. of sub-PMPs
    uint_t n; // sub-PMP size
    Semaphore sem;
    mutex weights_mutex;
    unordered_map<uint_t, double> weights; // spatial voting weights of N original locations
public:
    RSSV(const shared_ptr<Instance>& instance, uint_t seed, uint_t n);
    Solution run(int thread_cnt);
    void solveSubproblem(int seed);
    void processSubsolution(shared_ptr<Solution> solution);
    vector<uint_t> filterLocations(uint_t cnt);
    unordered_set<uint_t> extractPrioritizedLocations(uint_t min_cnt);
};

#endif //LARGE_PMP_RSSV_HPP
