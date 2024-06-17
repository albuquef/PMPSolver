#ifndef LARGE_PMP_TB_HPP
#define LARGE_PMP_TB_HPP

#include <omp.h>
#include <utility>
#include <sys/time.h>
#include <omp.h>


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
    bool cover_mode_n2=false;
    double time_limit=CLOCK_LIMIT;
public:
    explicit TB(shared_ptr<Instance> instance, uint_t seed);
    Solution_std initRandomSolution();
    Solution_std initRandomSolution_Cover();
    Solution_cap initRandomCapSolution();
    Solution_cap initHighestCapSolution();
    Solution_cap initHighestCapSolution_Cover();
    Solution_cap initSmartRandomCapSolution();
    Solution_cap initCPLEXCapSolution(double time_limit, const char* typeProb);
    Solution_cap fixedCapSolution(string eval_Method);

    Solution_std run(bool verbose, int MAX_ITE);
    Solution_cap run_cap(bool verbose, int MAX_ITE);
    Solution_std localSearch_std(Solution_std sol_best, bool verbose, int MAX_ITE);
    Solution_cap localSearch_cap(Solution_cap sol_best, bool verbose, int MAX_ITE);
    Solution_cap localSearch_cap_cover(Solution_cap sol_best, bool verbose, int MAX_ITE);
    // bool isBetterSolution(Solution_cap sol, uint_t in_p, uint_t out_p);   
    int isSolutionExistsinMap(Solution_cap sol, uint_t in_p, uint_t out_p);
    bool test_Capacity(Solution_cap sol, uint_t in_p, uint_t out_p);
    bool test_LB_PMP(Solution_cap sol, uint_t in_p, uint_t out_p);
    bool test_UB_heur(Solution_cap sol, uint_t in_p, uint_t out_p);
    bool test_Cover(unordered_set<uint_t> p_loc, uint_t in_p, uint_t out_p);
    bool test_CoverN2(unordered_set<uint_t> p_loc, uint_t in_p, uint_t out_p);
    bool test_SizeofP(unordered_set<uint_t> p_loc, uint_t in_p, uint_t out_p);
    bool test_basic_Solution_cap(Solution_cap sol, uint_t in_p, uint_t out_p);
    Solution_cap copySolution_cap(Solution_cap sol, bool createGAPeval=0);
    Solution_MAP solutions_map;
    void setSolutionMap(Solution_MAP sol_map);
    void setGenerateReports(bool generate_reports);
    void setMethod(string Method);
    void setExternalTime(double time);
    void setCoverMode(bool cover_mode);
    void setCoverMode_n2(bool cover_mode_n2);
    void setTimeLimit(double time_limit);
};


#endif //LARGE_PMP_TB_HPP