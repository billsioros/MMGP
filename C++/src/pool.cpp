
#include "pool.hpp"

Pool::Pool(std::size_t size)
{
    for (std::size_t id = 0; id < size; id++)
        jobs.emplace([&]()
        {
            for (;;)
            {
                std::function<void()> job;

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cnd.wait(lock, [&](){ return !jobs.empty(); });

                    job = jobs.front(); jobs.pop();
                }

                job();
            }
        });
}

Pool::~Pool()
{
    for (auto& t : threads)
        t.join();
}