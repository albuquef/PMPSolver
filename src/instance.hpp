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
    shared_ptr<dist_t[]> loc_capacities;
    shared_ptr<dist_t[]> dist_matrix;

    uint_t p;
    uint_t loc_max_id; // kept for addressing the full distance matrix
    uint_t cust_max_id; // kept for addressing the full distance matrix
    dist_t h; // bandwidth
    uint_t total_demand;
    const string type_service;

    void setDist(uint_t loc, uint_t cust, dist_t value);
public:
    Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> dist_matrix, shared_ptr<dist_t[]> loc_capacities, uint_t p, uint_t loc_max, uint_t cust_max,string type_service="null");    
    Instance(const string& dist_matrix_filename, const string& weights_filename, const string& capacities_filename, uint_t p, char delim, string type_service="null");
    
    dist_t getWeightedDist(uint_t loc, uint_t cust);
    dist_t getRealDist(uint_t loc, uint_t cust);
    dist_t getCustWeight(uint_t cust);
    Instance sampleSubproblem(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, uint_t seed);
    Instance getReducedSubproblem(const vector<uint_t>& locations_new, string type_service);
    void print();
    const vector<uint_t>& getCustomers() const;
    const vector<uint_t>& getLocations() const;
    uint_t get_p() const;
    uint_t getDistIndex(uint_t loc, uint_t cust);
    uint_t getClosestCust(uint_t loc);
    double getVotingScore(uint_t loc, uint_t cust);
    dist_t getLocCapacity(uint_t loc);
    dist_t getTotalDemand() const;
    string getTypeService() const;

};


#endif //LARGE_PMP_INSTANCE_HPP
