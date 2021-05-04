#ifndef LARGE_PMP_TB_HPP
#define LARGE_PMP_TB_HPP

#include "instance.hpp"
#include "solution.hpp"

using namespace std;

class TB {
private:
    Instance *instance;
    default_random_engine engine;
public:
    explicit TB(Instance *instance, uint_t seed);
    Solution initRandomSolution();
    Solution run(bool verbose);

};


#endif //LARGE_PMP_TB_HPP
