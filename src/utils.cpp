#include <iostream>
#include "utils.hpp"

chrono::steady_clock::time_point tick() {
    return chrono::steady_clock::now();
}

void tock(chrono::steady_clock::time_point start) {
    auto end = chrono::steady_clock::now();
    cout << "Elapsed time: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl << endl;
}

vector<uint_t> getRandomSubvector(vector<uint_t> orig_vector, uint_t new_size, default_random_engine *generator) {
    shuffle(orig_vector.begin(), orig_vector.end(), *generator);
    return vector<uint_t>(orig_vector.begin(), orig_vector.begin() + new_size);
}

bool sortbysec(const pair<int,int> &a, const pair<int,int> &b) {
    return (a.second < b.second);
}

bool cmpPair2nd(pair<uint_t, double>& a,
                pair<uint_t, double>& b)
{
    return a.second < b.second;
}
