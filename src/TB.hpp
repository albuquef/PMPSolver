#ifndef LARGE_PMP_TB_HPP
#define LARGE_PMP_TB_HPP

#include <omp.h>
#include <utility>

#include "instance.hpp"
#include "solution_std.hpp"
#include "solution_cap.hpp"
#include "globals.hpp"
#include "utils.hpp"


using namespace std;

class TB {
private:
    shared_ptr<Instance> instance;
    default_random_engine engine;
    string type_eval_solution;
public:
    explicit TB(shared_ptr<Instance> instance, uint_t seed);
    Solution_std initRandomSolution();
    Solution_cap initRandomCapSolution();
    Solution_cap initHighestCapSolution();

    Solution_std run(bool verbose, int MAX_ITE);
    Solution_cap run_cap(bool verbose, int MAX_ITE);
    Solution_std localSearch_std(Solution_std sol_best, bool verbose, int MAX_ITE);
    Solution_cap localSearch_cap(Solution_cap sol_best, bool verbose, int MAX_ITE);

};


#endif //LARGE_PMP_TB_HPP