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
#include "types.hpp"

class Instance {
private:
    vector<uint_t> locations;
    vector<uint_t> customers;
    shared_ptr<dist_t[]> dist_matrix;

    uint_t p;
    uint_t loc_max_id; // kept for addressing the full distance matrix
    uint_t cust_max_id; // kept for addressing the full distance matrix

    void setDist(uint_t loc, uint_t cust, dist_t value);
public:
    Instance(const string& loc_filename, const string& cust_filename, const string& dist_filename, uint_t p);
    Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> dist_matrix, uint_t p, uint_t loc_max, uint_t cust_max);

    dist_t getDist(uint_t loc, uint_t cust);
    Instance sampleSubproblem(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, default_random_engine *generator);
    void print();
    const vector<uint_t>& getCustomers() const;
};


#endif //LARGE_PMP_INSTANCE_HPP
