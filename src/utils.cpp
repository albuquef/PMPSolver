#include <iostream>
#include "utils.hpp"

chrono::steady_clock::time_point tick() {
    return chrono::steady_clock::now();
}

void tock(chrono::steady_clock::time_point start) {
    auto end = chrono::steady_clock::now();
    cout << "Elapsed time: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl << endl;
}
