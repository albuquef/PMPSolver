#include <iostream>
#include "utils.hpp"

chrono::steady_clock::time_point tick() {
    return chrono::steady_clock::now();
}

void tock(chrono::steady_clock::time_point start) {
    auto end = chrono::steady_clock::now();
    cout << "Elapsed time: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl << endl;
}

vector<uint_t> sampleSubvector(vector<uint_t> *orig_vector, uint_t max_value, uint_t new_size, default_random_engine *generator) {
    vector<bool> loc_flags(max_value + 1, false);
    vector<uint_t> new_vector;

    std::uniform_int_distribution<uint_t> distribution(0,orig_vector->size() - 1); // sample index in orig_vector
    uint_t cand;
    uint_t cnt = 0;
    while (cnt < new_size) {
        cand = orig_vector->at(distribution(*generator));
        if (!loc_flags[cand]) {
            cnt++;
            loc_flags[cand] = true;
            new_vector.push_back(cand);
        }
    }
    return new_vector;
}
