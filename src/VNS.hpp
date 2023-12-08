#ifndef LARGE_PMP_VNS_HPP
#define LARGE_PMP_VNS_HPP

#include <omp.h>
#include <utility>

#include "instance.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "TB.hpp"

using namespace std;

class VNS {
private:
    shared_ptr<Instance> instance;
    default_random_engine engine;
public:
    explicit VNS(shared_ptr<Instance> instance, uint_t seed);

    Solution_std rand_swap_Locations(Solution_std sol_current, int num_swaps);
    Solution_cap rand_swap_Locations_cap(Solution_cap sol_current, int num_swaps);

    Solution_std runVNS_std(bool verbose, int MAX_ITE);
    Solution_cap runVNS_cap(bool verbose, int MAX_ITE);
    // to do: add time limit
    // Solution_std runVNS_std(bool verbose, int MAX_ITE, int MAX_TIME);
    // Solution_cap runVNS_cap(bool verbose, int MAX_ITE, int MAX_TIME);

};


#endif //LARGE_PMP_VNS_HPP