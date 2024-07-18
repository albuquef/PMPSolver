#ifndef PMPSOLVER_VNS_HPP
#define PMPSOLVER_VNS_HPP

#include <omp.h>
#include <utility>
#include <string>
#include <sys/time.h>
#include <algorithm>

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
    bool cover_mode_n2=false;   
    double external_time=0;
    bool useInitSol=false;
    Solution_cap initial_solution;
public:
    explicit VNS(shared_ptr<Instance> instance, uint_t seed);

    template<typename SolutionType>
    SolutionType Swap_Locations(SolutionType sol_current, unsigned int num_swaps, int seed, bool capac = false);
    template<typename SolutionType>
    SolutionType Swap_Locations_Cover(SolutionType sol_current, unsigned int num_swaps, int seed, bool capac = false);


    Solution_std rand_swap_Locations(Solution_std sol_current, unsigned int num_swaps, int seed);
    Solution_std rand_swap_Locations_cover(Solution_std sol_current, unsigned int num_swaps, int seed);
    Solution_cap rand_swap_Locations_cap(Solution_cap sol_current, unsigned int num_swaps, int seed);
    Solution_cap rand_swap_Locations_cap_cover(Solution_cap sol_current, unsigned int num_swaps, int seed);

    // template<typename SolutionType>
    // SolutionType runVNS(SolutionType sol_current, bool verbose, int MAX_ITE, double EXTERNAL_TIME, bool capac = false);

    Solution_std runVNS_std(bool verbose, int MAX_ITE);
    Solution_cap runVNS_cap(const string& Method, bool verbose, int MAX_ITE);
    bool isBetter_cap(Solution_cap sol_cand, Solution_cap sol_best);    
    // to do: add time limit
    // Solution_std runVNS_std(bool verbose, int MAX_ITE, int MAX_TIME);
    // Solution_cap runVNS_cap(bool verbose, int MAX_ITE, int MAX_TIME);
    Solution_MAP solutions_map;
    void setSolutionMap(Solution_MAP sol_map);
    void setGenerateReports(bool generate_reports);
    void setMethod(string Method);
    void setCoverMode(bool cover_mode);
    void setCoverMode_n2(bool cover_mode_n2);
    void setExternalTime(double time);

    void setInitialSolution(Solution_cap sol);
    void setUseInitialSol(bool useInitSol);
};


#endif //PMPSOLVER_VNS_HPP