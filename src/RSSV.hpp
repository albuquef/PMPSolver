#ifndef LARGE_PMP_RSSV_HPP
#define LARGE_PMP_RSSV_HPP

#include <unordered_map>
#include <thread>
#include "TB.hpp"
#include "VNS.hpp"
#include "PMP.hpp"
#include "instance.hpp"
#include "instance.hpp"
#include "solution_std.hpp"
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
    string method_RSSV_sp;
    int DEFAULT_MAX_NUM_ITER = 10000000;
    bool cover_mode = false;
    bool cover_mode_n2 = false;
public:
    RSSV(const shared_ptr<Instance>& instance, uint_t seed, uint_t n);
    shared_ptr<Instance> run(uint_t thread_cnt, string& method_sp);
    shared_ptr<Instance> run_CAP(uint_t thread_cnt, string& method_sp);
    void solveSubproblem(int seed);
    void solveSubproblem_CAP(int seed);
    void processSubsolution(shared_ptr<Solution_std> solution);
    void processSubsolution_CAP(shared_ptr<Solution_cap> solution);
    vector<uint_t> filterLocations(uint_t cnt);
    unordered_set<uint_t> extractPrioritizedLocations(uint_t min_cnt);

    void setCoverMode(bool mode) {
        cover_mode = mode;
    }
    void setCoverMode_n2(bool mode) {
        cover_mode_n2 = mode;
    }
};

#endif //LARGE_PMP_RSSV_HPP