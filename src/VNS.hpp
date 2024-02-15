#ifndef LARGE_PMP_VNS_HPP
#define LARGE_PMP_VNS_HPP

#include <omp.h>
#include <utility>
#include <string>
#include <sys/time.h>

#include "instance.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "solution_map.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "TB.hpp"

using namespace std;

class VNS {
private:
    shared_ptr<Instance> instance;
    default_random_engine engine;
    bool generate_reports=false;
    string typeMethod="VNS";
    bool cover_mode=false;
public:
    explicit VNS(shared_ptr<Instance> instance, uint_t seed);

    

    Solution_std rand_swap_Locations(Solution_std sol_current, unsigned int num_swaps, int seed);
    Solution_cap rand_swap_Locations_cap(Solution_cap sol_current, unsigned int num_swaps, int seed);
    Solution_cap rand_swap_Locations_cap_cover(Solution_cap sol_current, unsigned int num_swaps, int seed);
    Solution_std runVNS_std(bool verbose, int MAX_ITE);
    Solution_cap runVNS_cap(string& Method, bool verbose, int MAX_ITE);
    bool isBetter_cap(Solution_cap sol_cand, Solution_cap sol_best);    
    // to do: add time limit
    // Solution_std runVNS_std(bool verbose, int MAX_ITE, int MAX_TIME);
    // Solution_cap runVNS_cap(bool verbose, int MAX_ITE, int MAX_TIME);
    Solution_MAP solutions_map;
    void setSolutionMap(Solution_MAP sol_map);
    void setGenerateReports(bool generate_reports);
    void setMethod(string Method);
    void setCoverMode(bool cover_mode);
};


#endif //LARGE_PMP_VNS_HPP