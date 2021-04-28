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
    sol.print();
    sem.notify(seed);
}


