
#ifndef __POOL__
#define __POOL__

#include <functional>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>

class Pool
{
    std::vector<std::thread> threads;

    static std::mutex mtx;
    static std::condition_variable cnd;
    static std::queue<std::function<void()>> jobs;

public:

    Pool(std::size_t);
    ~Pool();
};

#endif
