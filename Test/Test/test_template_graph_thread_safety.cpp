#include "../../Template/graph.hpp"
#include "../../Template/vector.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <atomic>
#include <chrono>
#include <thread>

FT_TEST(test_ft_graph_enable_thread_safety_controls,
        "ft_graph installs optional mutex guards and protects operations")
{
    ft_graph<int> graph_instance;
    ft_vector<size_t> neighbor_list;
    size_t vertex_a;
    size_t vertex_b;
    bool lock_acquired;

    FT_ASSERT_EQ(0, graph_instance.enable_thread_safety());
    FT_ASSERT(graph_instance.is_thread_safe());

    lock_acquired = false;
    FT_ASSERT_EQ(0, graph_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    graph_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(ER_SUCCESS, graph_instance.get_error());

    vertex_a = graph_instance.add_vertex(10);
    vertex_b = graph_instance.add_vertex(20);
    graph_instance.add_edge(vertex_a, vertex_b);
    FT_ASSERT_EQ(ER_SUCCESS, graph_instance.get_error());

    graph_instance.neighbors(vertex_a, neighbor_list);
    FT_ASSERT_EQ(ER_SUCCESS, graph_instance.get_error());
    FT_ASSERT(neighbor_list.size() == 1);
    FT_ASSERT(neighbor_list[0] == vertex_b);

    graph_instance.disable_thread_safety();
    FT_ASSERT(graph_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, graph_instance.get_error());

    graph_instance.clear();
    return (1);
}

FT_TEST(test_ft_graph_lock_blocks_until_release,
        "ft_graph lock waits for mutex holders to release")
{
    ft_graph<int> graph_instance;
    bool main_lock_acquired;
    std::atomic<bool> ready;
    std::atomic<bool> worker_succeeded;
    std::atomic<long long> wait_duration_ms;
    std::thread worker_thread;

    FT_ASSERT_EQ(0, graph_instance.enable_thread_safety());
    FT_ASSERT(graph_instance.is_thread_safe());

    main_lock_acquired = false;
    FT_ASSERT_EQ(0, graph_instance.lock(&main_lock_acquired));
    FT_ASSERT(main_lock_acquired == true);

    ready.store(false);
    worker_succeeded.store(false);
    wait_duration_ms.store(0);
    worker_thread = std::thread([&graph_instance, &ready, &worker_succeeded, &wait_duration_ms]() {
        bool worker_lock_acquired;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::steady_clock::time_point end_time;

        worker_lock_acquired = false;
        ready.store(true);
        start_time = std::chrono::steady_clock::now();
        if (graph_instance.lock(&worker_lock_acquired) != 0)
        {
            worker_succeeded.store(false);
            wait_duration_ms.store(-1);
            return ;
        }
        end_time = std::chrono::steady_clock::now();
        wait_duration_ms.store(std::chrono::duration_cast<std::chrono::milliseconds>(
                    end_time - start_time).count());
        worker_succeeded.store(worker_lock_acquired);
        graph_instance.unlock(worker_lock_acquired);
    });

    while (!ready.load())
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    graph_instance.unlock(main_lock_acquired);

    worker_thread.join();

    FT_ASSERT(worker_succeeded.load());
    FT_ASSERT(wait_duration_ms.load() >= 40);

    graph_instance.disable_thread_safety();
    FT_ASSERT(graph_instance.is_thread_safe() == false);
    FT_ASSERT_EQ(ER_SUCCESS, graph_instance.get_error());

    graph_instance.clear();
    return (1);
}
