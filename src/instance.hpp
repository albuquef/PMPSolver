#ifndef LARGE_PMP_INSTANCE_HPP
#define LARGE_PMP_INSTANCE_HPP

#include <set>
#include <utility>
#include <vector>
#include <memory>
#include "types.hpp"

class Instance {
private:
    set<uint_t> locations;
    set<uint_t> customers;
    uint_t p;
    uint_t loc_max;
    uint_t cust_max;
    dist_t *dist_matrix;

    void setDist(uint_t loc, uint_t cust, dist_t value);
public:
    Instance(string loc_filename, string cust_filename, string dist_filename, uint_t p);
    ~Instance();
    dist_t getDist(uint_t loc, uint_t cust);
};


#endif //LARGE_PMP_INSTANCE_HPP
