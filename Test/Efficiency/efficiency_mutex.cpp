#include "../../PThread/mutex.hpp"
#include "../../PThread/pthread.hpp"
#include "utils.hpp"

#include <mutex>
#include <thread>
#include <vector>

int test_efficiency_mutex_lock(void)
{
    const int iterations = 10000;
    const int thread_count = 4;

    std::mutex std_mtx;
    pt_mutex ft_mtx;

    auto std_worker = [&]() {
        for (int i = 0; i < iterations; ++i)
        {
            std_mtx.lock();
            prevent_optimization((void*)&std_mtx);
            std_mtx.unlock();
        }
    };

    auto ft_worker = [&]() {
        pt_thread_id_type id = THREAD_ID;
        for (int i = 0; i < iterations; ++i)
        {
            ft_mtx.lock(id);
            prevent_optimization((void*)&ft_mtx);
            ft_mtx.unlock(id);
        }
    };

    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    auto start_std = clock_type::now();
    for (int i = 0; i < thread_count; ++i)
        threads.emplace_back(std_worker);
    for (std::thread &t : threads)
        t.join();
    auto end_std = clock_type::now();

    threads.clear();

    auto start_ft = clock_type::now();
    for (int i = 0; i < thread_count; ++i)
        threads.emplace_back(ft_worker);
    for (std::thread &t : threads)
        t.join();
    auto end_ft = clock_type::now();

    print_comparison("mutex lock/unlock", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}
