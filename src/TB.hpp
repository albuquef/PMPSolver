#ifndef LARGE_PMP_TB_HPP
#define LARGE_PMP_TB_HPP

#include "instance.hpp"
#include "solution_std.hpp"

using namespace std;

class TB {
private:
    shared_ptr<Instance> instance;
    default_random_engine engine;
public:
    explicit TB(shared_ptr<Instance> instance, uint_t seed);
    Solution_std initRandomSolution();
    Solution_std run(bool verbose);

};


#endif //LARGE_PMP_TB_HPP
