#ifndef PMPSOLVER_RSSV_HPP
#define PMPSOLVER_RSSV_HPP

#include <unordered_map>
#include <thread>
#include <functional>
#include <mutex>
#include <chrono>
#include "TB.hpp"
#include "VNS.hpp"
#include "PMP.hpp"
#include "instance.hpp"
#include "instance.hpp"
#include "solution_std.hpp"
#include "semaphore.hpp"

using namespace std;
using namespace chrono;

class RSSV {
private:
    shared_ptr<Instance> instance; // original PMP instance
    default_random_engine engine;
    int seed_rssv;
    uint_t N; // original PMP size (no. of locations)
    uint_t M; // no. of sub-PMPs
    uint_t n; // sub-PMP size
    uint_t n_cand; // no. of candidate locations
    uint_t num_customers_subproblem;
    uint_t num_facilities_subproblem;
    uint_t p_subproblem;
    Semaphore sem;
    mutex weights_mutex;
    mutex dist_mutex;
    mutex cap_check_mutex;
    unordered_map<uint_t, double> weights; // spatial voting weights of N original locations
    vector<uint_t> final_voted_locs; // locations that are filtered 
    string method_RSSV_sp;
    int DEFAULT_MAX_NUM_ITER = 10000000;
    bool add_threshold_dist = false;
    bool cover_mode = false;
    bool cover_mode_n2 = false;
    mutex mtx;
    dist_t subSols_max_dist=0;
    dist_t subSols_minmax_dist=numeric_limits<dist_t>::max();;
    dist_t subSols_min_dist=numeric_limits<dist_t>::max();;
    dist_t subSols_avg_dist=0;
    dist_t subSols_std_dev_dist=0; 
    uint_t subSols_max_num_assignments=0;

    uint_t MAX_ITE_SUBPROBLEMS = 0;
    dist_t TIME_LIMIT_SUBPROBLEMS = 0;


    bool capacities_respected = false;
    dist_t max_dist_feas = numeric_limits<dist_t>::max();;

public:
    RSSV(const shared_ptr<Instance>& instance, uint_t seed, uint_t n, uint_t n_cand=0);
    shared_ptr<Instance> run(uint_t thread_cnt, const string& method_sp);
    shared_ptr<Instance> run_CAP(uint_t thread_cnt, const string& method_sp);
    shared_ptr<Instance> run_impl(uint_t thread_cnt, const string& method_sp, bool is_cap);
    template <typename SolutionType>
    void solveSubproblemTemplate(int seed, bool isCapacitated);
    template <typename SolutionType>
    void processSubsolutionScores(shared_ptr<SolutionType> solution);
    template <typename SolutionType>
    void processSubsolutionDists(shared_ptr<SolutionType> solution);
    template <typename SolutionType>
    void processSubsolutionCapacities(shared_ptr<SolutionType> solution);
    vector<uint_t> filterLocations(uint_t cnt);
    vector<uint_t> filterLocations_nonzero(uint_t cnt);
    vector<uint_t> randomLocations(uint_t cnt);
    unordered_set<uint_t> extractPrioritizedLocations(uint_t min_cnt);
    vector<uint_t> extractFixedLocations(vector<uint_t> vet_locs);
    void setTIME_LIMIT_SUBPROBLEMS(dist_t time_limit);
    void setMAX_ITE_SUBPROBLEMS(uint_t max_ite);
    vector<uint_t> getFinalVotedLocs();

    void setCoverMode(bool mode) {
        cover_mode = mode;
    }
    void setCoverMode_n2(bool mode) {
        cover_mode_n2 = mode;
    }
    void setAddThresholdDist(bool add_threshold) {
        add_threshold_dist = add_threshold;
    }
};

#endif //PMPSOLVER_RSSV_HPP