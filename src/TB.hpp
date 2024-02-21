#ifndef LARGE_PMP_TB_HPP
#define LARGE_PMP_TB_HPP

#include <omp.h>
#include <utility>
#include <sys/time.h>

#include "instance.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "solution_map.hpp"
#include "globals.hpp"
#include "utils.hpp"
#include "PMP.hpp"


using namespace std;

class TB {
private:
    shared_ptr<Instance> instance;
    default_random_engine engine;
    string type_eval_solution;
    bool generate_reports=false;
    string typeMethod="TB";
    double external_time=0;
    bool cover_mode=false;
public:
    explicit TB(shared_ptr<Instance> instance, uint_t seed);
    Solution_std initRandomSolution();
    Solution_std initRandomSolution_cover();
    Solution_cap initRandomCapSolution();
    Solution_cap initHighestCapSolution();
    Solution_cap initHighestCapSolution_Cover();
    Solution_cap initSmartRandomCapSolution();
    Solution_cap initCPLEXCapSolution(double time_limit);

    Solution_std run(bool verbose, int MAX_ITE);
    Solution_cap run_cap(bool verbose, int MAX_ITE);
    Solution_std localSearch_std(Solution_std sol_best, bool verbose, int MAX_ITE);
    Solution_cap localSearch_cap(Solution_cap sol_best, bool verbose, int MAX_ITE);
    Solution_cap localSearch_cap_cover(Solution_cap sol_best, bool verbose, int MAX_ITE);
    // bool isBetterSolution(Solution_cap sol, uint_t in_p, uint_t out_p);   
    int isSolutionExistsinMap(Solution_cap sol, uint_t in_p, uint_t out_p);
    bool test_Capacity(Solution_cap sol, uint_t in_p, uint_t out_p);
    bool test_LB_PMP(Solution_cap sol, uint_t in_p, uint_t out_p);
    bool test_Cover(uint_t in_p, uint_t out_p);
    Solution_MAP solutions_map;
    void setSolutionMap(Solution_MAP sol_map);
    void setGenerateReports(bool generate_reports);
    void setMethod(string Method);
    void setExternalTime(double time);
    void setCoverMode(bool cover_mode);
};


#endif //LARGE_PMP_TB_HPP