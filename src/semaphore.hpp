#ifndef PMPSOLVER_SEMAPHORE_HPP
#define PMPSOLVER_SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore (int count_ = 0)
            : count(count_)
    {
    }

    inline void setCount(int count_) {
        count = count_;
    }

    inline void notify( int tid ) {
        this->tid = tid;
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        //cout << "thread " << tid <<  " notify" << endl;
        //notify the waiting thread
        cv.notify_one();
    }
    inline void wait( int tid ) {
        this->tid = tid;
        std::unique_lock<std::mutex> lock(mtx);
        while(count == 0) {
            //cout << "thread " << tid << " wait" << endl;
            //wait on the mutex until notify is called
            cv.wait(lock);
            //cout << "thread " << tid << " run" << endl;
        }
        count--;
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    int tid;
    int count;
};

#endif //PMPSOLVER_SEMAPHORE_HPP