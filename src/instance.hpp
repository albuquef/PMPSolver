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
#include <limits.h>
#include <algorithm>
#include <math.h>
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
    uint_t cover_max_id=0; 
    uint_t cover_n2_max_id=0; 
    dist_t h; // bandwidth
    uint_t total_demand;
    const string type_service;
    vector<uint_t> voted_locs;

    unordered_set<uint_t> unique_subareas;
    shared_ptr<uint_t[]> loc_coverages;
    string type_subarea="null";
    bool cover_mode=false;
    unordered_set<uint_t> unique_subareas_n2;
    shared_ptr<uint_t[]> loc_coverages_n2;
    string type_subarea_n2="null";
    bool cover_mode_n2=false;

    vector<pair<dist_t, dist_t>> loc_coordinates;
    vector<pair<dist_t, dist_t>> cust_coordinates;

    bool is_weighted_obj_func = true;

    void setDist(uint_t loc, uint_t cust, dist_t value);
    dist_t threshold_dist=0;
public:
    // Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> dist_matrix, shared_ptr<dist_t[]> loc_capacities, uint_t p, uint_t loc_max, uint_t cust_max, string type_service);    
    Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> loc_capacities,shared_ptr<dist_t[]> dist_matrix, uint_t p, uint_t loc_max, uint_t cust_max, string type_service);    
    Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> loc_capacities,shared_ptr<dist_t[]> dist_matrix, uint_t p, uint_t loc_max, uint_t cust_max, string type_service, unordered_set<uint_t> unique_subareas, shared_ptr<uint_t[]> loc_coverages, string type_subarea);
    Instance(vector<uint_t> locations, vector<uint_t> customers, shared_ptr<dist_t[]> cust_weights, shared_ptr<dist_t[]> loc_capacities,shared_ptr<dist_t[]> dist_matrix, uint_t p, uint_t loc_max, uint_t cust_max, string type_service, unordered_set<uint_t> unique_subareas, shared_ptr<uint_t[]> loc_coverages, string type_subarea, unordered_set<uint_t> unique_subareas_n2, shared_ptr<uint_t[]> loc_coverages_n2, string type_subarea_n2);
    Instance(const string& dist_matrix_filename, const string& weights_filename, const string& capacities_filename, uint_t p, char delim, string type_service="null",uint_t cust_max_id=0, uint_t loc_max_id=0);
    Instance(uint_t cust_max_id, uint_t loc_max_id, const string& weights_filename, const string& capacities_filename, uint_t p, char delim, string type_service="null");
    
    dist_t getWeightedDist(uint_t loc, uint_t cust);
    dist_t getRealDist(uint_t loc, uint_t cust);
    dist_t getCustWeight(uint_t cust);
    Instance sampleSubproblem(uint_t loc_cnt, uint_t cust_cnt, uint_t p_new, uint_t seed);
    Instance getReducedSubproblem(const vector<uint_t>& locations_new, string type_service);
    Instance filterInstance(string type_service);
    // Instance getReducedSubproblem(const vector<uint_t>& locations_new, string type_service, unordered_set<uint_t> unique_subareas, shared_ptr<uint_t[]> loc_coverages, string type_subarea);
    void print();
    const vector<uint_t>& getCustomers() const;
    const vector<uint_t>& getLocations() const;
    uint_t get_p() const;
    uint_t getDistIndex(uint_t loc, uint_t cust);
    uint_t getLocIndex(uint_t loc);
    uint_t getCustIndex(uint_t cust);
    uint_t getClosestCust(uint_t loc);
    double getVotingScore(uint_t loc, uint_t cust);
    dist_t getLocCapacity(uint_t loc);
    dist_t getTotalDemand() const;
    string getTypeService() const;
    void setVotedLocs(vector<uint_t> voted_locs);
    vector<uint_t> getVotedLocs();

    void ReadCoverages(const string& coverages_filename, const string type_subarea,char delim);
    void ReadCoverages_n2(const string& coverages_filename_n2, const string type_subarea_n2,char delim);
    string getTypeSubarea() const;
    string getTypeSubarea_n2() const;
    uint_t getSubareaLocation(uint_t loc);
    uint_t getSubareaLocation_n2(uint_t loc);
    bool isInTheSameSubarea(uint_t loc1, uint_t loc2);
    bool isInTheSameSubarea_n2(uint_t loc1, uint_t loc2);
    const vector<uint_t> getLocationsSubarea(uint_t subarea);
    const vector<uint_t> getLocationsSubarea_n2(uint_t subarea);
    unordered_set<uint_t>  getSubareasSet();
    unordered_set<uint_t>  getSubareasSet_n2();
    void setCoverModel(bool cover_mode);
    void setCoverModel_n2(bool cover_mode_n2);
    bool isCoverMode();
    bool isCoverMode_n2();
    bool isPcoversAllSubareas(unordered_set<uint_t> p_loc_cand);
    bool isPcoversAllSubareas_n2(unordered_set<uint_t> p_loc_cand);
    bool get_isWeightedObjFunc();
    void set_isWeightedObjFunc(bool is_weighted_obj_func);
    void set_ThresholdDist(dist_t threshold_dist);
    dist_t get_ThresholdDist();

};


#endif //LARGE_PMP_INSTANCE_HPP
