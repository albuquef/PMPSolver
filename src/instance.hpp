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

    shared_ptr<uint_t[]> loc_coverages;
    unordered_set<uint_t> unique_subareas;

    uint_t p;
    uint_t loc_max_id; // kept for addressing the full distance matrix
    uint_t cust_max_id; // kept for addressing the full distance matrix
    uint_t cover_max_id=0; 
    dist_t h; // bandwidth
    uint_t total_demand;

    const string type_service;
    string type_subarea;
    bool cover_mode=false;

    void setDist(uint_t loc, uint_t cust, dist_t value);
public:
    // Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> dist_matrix, shared_ptr<dist_t[]> loc_capacities, uint_t p, uint_t loc_max, uint_t cust_max, string type_service);    
    Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> loc_capacities,shared_ptr<dist_t[]> dist_matrix, uint_t p, uint_t loc_max, uint_t cust_max, string type_service);    
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
    

    void ReadCoverages(const string& coverages_filename, const string type_subarea,char delim);
    string getTypeSubarea() const;
    uint_t getSubareaLocation(uint_t loc);
    bool isInTheSameSubarea(uint_t loc1, uint_t loc2);
    const vector<uint_t> getLocationsSubarea(uint_t subarea);
    unordered_set<uint_t>  getSubareasSet();
    void setCoverModel(bool cover_mode);
    bool isCoverMode();


};


#endif //LARGE_PMP_INSTANCE_HPP
