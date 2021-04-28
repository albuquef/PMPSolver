#include <thread>
#include <unistd.h>
#include "RSSV.hpp"
#include "TB.hpp"

RSSV::RSSV(Instance *instance, uint_t seed, uint_t n):instance(instance), n(n) {
    engine.seed(seed);
    N = instance->getLocations().size();
    M = 5*N/n;

    for (auto loc : instance->getLocations()) {
        weights[loc] = 0;
    }
}

Solution RSSV::run() {
    cout << "RSSV running...\n";
    cout << "PMP size (N): " << N << endl;
    cout << "sub-PMP size (n): " << n << endl;
    cout << "Subproblems cnt (M): " << M << endl;


    uint_t thread_cnt = 4;
    sem.setCount(thread_cnt);

    vector<thread> threads;
    for (uint_t i = 1; i <= M; i++) {
        threads.emplace_back(&RSSV::solveSubproblem, this, i);
    }

    for (auto &th:threads) {
        th.join();
    }

    return Solution();
}

void RSSV::solveSubproblem(uint_t seed) {
    sem.wait(seed);
    cout << "Solving sub-PMP " << seed << "/" << M << endl;
    Instance subInstance = instance->sampleSubproblem(n, n, instance->get_p(), &engine);
//    subInstance.print();
    TB heuristic(&subInstance, seed);
    auto sol = heuristic.run();

    cout << "Solution " << seed << endl;
    processSubsolution(&sol);
    sem.notify(seed);
}

void RSSV::processSubsolution(Solution *solution) {
    solution->print();

    // todo use real distance here
    // todo estimate distance from closest customer
    for (auto loc_sol:solution->get_pLocations()) {
        cout << "loc_sol: " << loc_sol << endl;

        dist_t dist_min = numeric_limits<dist_t>::max();
        uint_t cust_cl;
        for (auto cust:solution->instance->getCustomers()) {
            auto dist = solution->instance->getRealDist(loc_sol, cust);
            if (dist <= dist_min) {
                dist_min = dist;
                cust_cl = cust;
            }
        }

        cout << "cust_cl: " << cust_cl << endl;
        cout << "dist_min: " << dist_min << endl;

    }
//    for (auto loc_cand:solution->instance->getLocations()) {
//        for (auto loc_sol:solution->get_pLocations()) {
//
//        }
//    }
}


