#include <cstdlib>
#include <ctime>  // clock
#include <iostream>
#include "globals.hpp"
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

unsigned int getAvailableThreads(void) {
    unsigned int availableThreads =  std::thread::hardware_concurrency();

    if (availableThreads <= 0) {
        return 1;
    }

    return availableThreads;
}

void setThreadNumber(const int number) {
    if (number < 0) {
        std::cerr << "Invalid number of threads." << std::endl;
        exit(1);
    } else if (number == 0) {
        THREAD_NUMBER = 1;
        return;
    }

    THREAD_NUMBER = number;
}

void setClockLimit(const uint_t limit) {
    CLOCK_LIMIT = limit;
}

void checkClock(void) {
    clock_t clock_current = clock();

    if (CLOCK_THREADED) {
        CLOCK_ELAPSED += ((clock_current / CLOCKS_PER_SEC) - CLOCK_ELAPSED) / THREAD_NUMBER;
    } else {
        CLOCK_ELAPSED = (clock_current / CLOCKS_PER_SEC - CLOCK_START);
    }

    if (CLOCK_ELAPSED >= CLOCK_LIMIT) {
        std::cerr << "Time limit exceeded. It took more than " << clock_current / CLOCKS_PER_SEC << "s to finish. You should allocate more time using \"-t <time.s>\" option." << std::endl;

        exit(1);
    }
}
