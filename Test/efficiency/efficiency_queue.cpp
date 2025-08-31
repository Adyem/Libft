#include "../../Template/queue.hpp"
#include "../../Template/swap.hpp"
#include "efficiency_utils.hpp"

#include <queue>

int test_efficiency_queue_enqueue_dequeue(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::queue<int> q;
        for (size_t i = 0; i < iterations; ++i)
            q.push(static_cast<int>(i));
        while (!q.empty())
        {
            sum += q.front();
            q.pop();
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_queue<int> q;
        for (size_t i = 0; i < iterations; ++i)
            q.enqueue(static_cast<int>(i));
        while (q.size() > 0)
        {
            sum += q.front();
            q.dequeue();
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("queue enqueue/dequeue", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_queue_interleaved(void)
{
    const size_t iterations = 100000;
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    {
        std::queue<int> q;
        for (size_t i = 0; i < iterations; ++i)
        {
            q.push(static_cast<int>(i));
            sum += q.front();
            q.pop();
        }
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    {
        ft_queue<int> q;
        for (size_t i = 0; i < iterations; ++i)
        {
            q.enqueue(static_cast<int>(i));
            sum += q.dequeue();
        }
    }
    auto end_ft = clock_type::now();

    prevent_optimization((void*)&sum);
    print_comparison("queue interleaved", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_queue_move(void)
{
    const size_t elements = 100000;
    std::queue<int> std_src;
    ft_queue<int> ft_src;
    for (size_t i = 0; i < elements; ++i)
    {
        std_src.push(static_cast<int>(i));
        ft_src.enqueue(static_cast<int>(i));
    }
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    std::queue<int> std_moved(std::move(std_src));
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    ft_queue<int> ft_moved(std::move(ft_src));
    auto end_ft = clock_type::now();

    while (!std_moved.empty())
    {
        sum += std_moved.front();
        std_moved.pop();
    }
    while (ft_moved.size() > 0)
    {
        sum += ft_moved.front();
        ft_moved.dequeue();
    }
    prevent_optimization((void*)&sum);

    print_comparison("queue move", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    return (1);
}

int test_efficiency_queue_swap(void)
{
    const size_t elements = 100000;
    const size_t iterations = 1000;
    std::queue<int> std_a, std_b;
    ft_queue<int> ft_a, ft_b;

    for (size_t i = 0; i < elements; ++i)
    {
        std_a.push(static_cast<int>(i));
        std_b.push(static_cast<int>(elements - i));
        ft_a.enqueue(static_cast<int>(i));
        ft_b.enqueue(static_cast<int>(elements - i));
    }
    volatile long long sum = 0;

    auto start_std = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&std_a);
        prevent_optimization((void*)&std_b);
        std::swap(std_a, std_b);
    }
    auto end_std = clock_type::now();

    auto start_ft = clock_type::now();
    for (size_t i = 0; i < iterations; ++i)
    {
        prevent_optimization((void*)&ft_a);
        prevent_optimization((void*)&ft_b);
        ft_swap(ft_a, ft_b);
    }
    auto end_ft = clock_type::now();

    sum += std_a.front();
    sum += std_b.front();
    sum += ft_a.front();
    sum += ft_b.front();
    prevent_optimization((void*)&sum);

    print_comparison("queue swap", elapsed_us(start_std, end_std),
                     elapsed_us(start_ft, end_ft));
    if (std_a.size() == ft_a.size() && std_b.size() == ft_b.size())
        return (1);
    return (0);
}

