#ifndef LARGE_PMP_INSTANCE_HPP
#define LARGE_PMP_INSTANCE_HPP

#include <set>
#include <unordered_set>
#include <utility>
#include <vector>
#include <memory>
#include "types.hpp"

class Instance {
private:
    vector<uint_t> locations;
    vector<uint_t> customers;
    shared_ptr<dist_t[]> dist_matrix;

    uint_t p;
    uint_t loc_max;
    uint_t cust_max;

    void setDist(uint_t loc, uint_t cust, dist_t value);
public:
    Instance(const string& loc_filename, const string& cust_filename, const string& dist_filename, uint_t p);

    dist_t getDist(uint_t loc, uint_t cust);
};


#endif //LARGE_PMP_INSTANCE_HPP
