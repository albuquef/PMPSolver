#ifndef LARGE_PMP_INSTANCE_HPP
#define LARGE_PMP_INSTANCE_HPP
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <utility>
#include <random>
#include "utils.hpp"
#include "globals.hpp"

class Instance {
private:
    vector<uint_t> locations;
    vector<uint_t> customers;
    shared_ptr<dist_t[]> cust_weights;
    shared_ptr<dist_t[]> dist_matrix;

    uint_t p;
    uint_t loc_max_id; // kept for addressing the full distance matrix
    uint_t cust_max_id; // kept for addressing the full distance matrix
    dist_t h; // bandwidth

    void setDist(uint_t loc, uint_t cust, dist_t value);
public:
    Instance(const string& loc_filename, const string& cust_filename, const string& dist_filename, const string& weights_filename, uint_t p);
    Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> dist_matrix, uint_t p, uint_t loc_max, uint_t cust_max);

    dist_t getWeightedDist(uint_t loc, uint_t cust);
    dist_t getRealDist(uint_t loc, uint_t cust);
    dist_t getCustWeight(uint_t cust);
    Instance sampleSubproblem(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, default_random_engine *generator);
    Instance getReducedSubproblem(const vector<uint_t>& locations_new);
    void print();
    const vector<uint_t>& getCustomers() const;
    const vector<uint_t>& getLocations() const;
    uint_t get_p() const;
    uint_t getDistIndex(uint_t loc, uint_t cust);
    uint_t getClosestCust(uint_t loc);
    double getVotingScore(uint_t loc, uint_t cust);

};


#endif //LARGE_PMP_INSTANCE_HPP
